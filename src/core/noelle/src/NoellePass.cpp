/*
 * Copyright 2023 - 2024  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "arcana/noelle/core/NoellePass.hpp"
#include "arcana/noelle/core/Architecture.hpp"

namespace arcana::noelle {

static cl::opt<int> Verbose(
    "noelle-verbose",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Verbose output (0: disabled, 1: minimal, 2: maximal)"));
static cl::opt<int> MinimumHotness(
    "noelle-min-hot",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Minimum hotness of code to be parallelized"));
static cl::opt<int> MaximumCores(
    "noelle-max-cores",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Maximum number of logical cores that Noelle can use"));
static cl::opt<bool> ND_PRVGs("noelle-nondeterministic-prvgs",
                              cl::ZeroOrMore,
                              cl::Hidden,
                              cl::desc("Consider PRVGs nondeterministic"));
static cl::opt<bool> DisableFloatAsReal(
    "noelle-disable-float-as-real",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Do not consider floating point variables as real numbers"));
static cl::opt<bool> DisableDSWP("noelle-disable-dswp",
                                 cl::ZeroOrMore,
                                 cl::Hidden,
                                 cl::desc("Disable DSWP"));
static cl::opt<bool> DisableHELIX("noelle-disable-helix",
                                  cl::ZeroOrMore,
                                  cl::Hidden,
                                  cl::desc("Disable HELIX"));
static cl::opt<bool> DisableDOALL("noelle-disable-doall",
                                  cl::ZeroOrMore,
                                  cl::Hidden,
                                  cl::desc("Disable DOALL"));
static cl::opt<bool> DisableDistribution(
    "noelle-disable-loop-distribution",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Disable the loop distribution"));
static cl::opt<bool> DisableInvCM(
    "noelle-disable-loop-invariant-code-motion",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Disable the loop invariant code motion"));
static cl::opt<bool> DisableWhilifier("noelle-disable-whilifier",
                                      cl::ZeroOrMore,
                                      cl::Hidden,
                                      cl::desc("Disable the loop whilifier"));
static cl::opt<bool> DisableSCEVSimplification(
    "noelle-disable-scev-simplification",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Disable IV related SCEV simplification"));
static cl::opt<bool> DisableLoopAwareDependenceAnalyses(
    "noelle-disable-loop-aware-dependence-analyses",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Disable loop aware dependence analyses"));
static cl::opt<bool> DisableInliner("noelle-disable-inliner",
                                    cl::ZeroOrMore,
                                    cl::Hidden,
                                    cl::desc("Disable the function inliner"));
static cl::opt<bool> InlinerDisableHoistToMain(
    "noelle-inliner-avoid-hoist-to-main",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Disable the function inliner"));

static cl::opt<int> PDGVerbose(
    "noelle-pdg-verbose",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc(
        "Verbose output (0: disabled, 1: minimal, 2: maximal, 3:maximal plus dumping PDG"));

static cl::opt<bool> PDGDump("noelle-pdg-dump",
                             cl::ZeroOrMore,
                             cl::Hidden,
                             cl::desc("Dump the PDG"));

static cl::opt<bool> PDGCheck("noelle-pdg-check",
                              cl::ZeroOrMore,
                              cl::Hidden,
                              cl::desc("Check the PDG"));

static cl::opt<bool> PDGSVFDisable("noelle-disable-pdg-svf",
                                   cl::ZeroOrMore,
                                   cl::Hidden,
                                   cl::desc("Disable SVF"));

static cl::opt<bool> PDGSVFCallGraphDisable("noelle-disable-pdg-svf-callgraph",
                                            cl::ZeroOrMore,
                                            cl::Hidden,
                                            cl::desc("Disable SVF call graph"));

static cl::opt<bool> PDGAllocAADisable(
    "noelle-disable-pdg-allocaa",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Disable our custom alias analysis"));

static cl::opt<bool> PDGRADisable(
    "noelle-disable-pdg-reaching-analysis",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Disable the use of reaching analysis to compute the PDG"));

NoellePass::NoellePass() : ModulePass{ ID }, n{ nullptr } {

  return;
}

bool NoellePass::doInitialization(Module &M) {
  return false;
}

void NoellePass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BlockFrequencyInfoWrapperPass>();
  AU.addRequired<BranchProbabilityInfoWrapperPass>();
  AU.addRequired<AssumptionCacheTracker>();
  AU.addRequired<CallGraphWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();

  return;
}

bool NoellePass::runOnModule(Module &M) {
  std::unordered_set<Transformation> enabledTransformations;
  Verbosity verbose = Verbosity::Disabled;
  double minHot = 0.0;
  LDGGenerator ldgGenerator;
  CompilationOptionsManager *om;

  /*
   * Default configuration
   */
  for (auto i = (uint32_t)Transformation::First;
       i <= (uint32_t)Transformation::Last;
       i++) {
    auto transformationID = static_cast<Transformation>(i);
    enabledTransformations.insert(transformationID);
  }

  /*
   * Fetch the command line options.
   */
  verbose = static_cast<Verbosity>(Verbose.getValue());
  minHot = ((double)(MinimumHotness.getValue())) / 1000;
  auto optMaxCores = MaximumCores.getValue();
  if (optMaxCores == 0) {
    optMaxCores = Architecture::getNumberOfPhysicalCores();
  }
  if (DisableDOALL.getNumOccurrences() > 0) {
    enabledTransformations.erase(DOALL_ID);
  }
  if (DisableDSWP.getNumOccurrences() > 0) {
    enabledTransformations.erase(DSWP_ID);
  }
  if (DisableHELIX.getNumOccurrences() > 0) {
    enabledTransformations.erase(HELIX_ID);
  }
  if (DisableDistribution.getNumOccurrences() > 0) {
    enabledTransformations.erase(LOOP_DISTRIBUTION_ID);
  }
  if (DisableInvCM.getNumOccurrences() > 0) {
    enabledTransformations.erase(LOOP_INVARIANT_CODE_MOTION_ID);
  }
  if (DisableWhilifier.getNumOccurrences() > 0) {
    enabledTransformations.erase(LOOP_WHILIFIER_ID);
  }
  if (DisableSCEVSimplification.getNumOccurrences() > 0) {
    enabledTransformations.erase(SCEV_SIMPLIFICATION_ID);
  }
  if (DisableInliner.getNumOccurrences() > 0) {
    enabledTransformations.erase(INLINER_ID);
  }
  if (DisableLoopAwareDependenceAnalyses.getNumOccurrences() == 0) {
    ldgGenerator.enableLoopDependenceAnalyses(true);
  } else {
    ldgGenerator.enableLoopDependenceAnalyses(false);
  }
  auto pdgVerbose = static_cast<PDGVerbosity>(PDGVerbose.getValue());
  auto dumpPDG = (PDGDump.getNumOccurrences() > 0) ? true : false;
  auto performThePDGComparison =
      (PDGCheck.getNumOccurrences() > 0) ? true : false;
  auto disableSVF = (PDGSVFDisable.getNumOccurrences() > 0) ? true : false;
  auto disableSVFCallGraph =
      (PDGSVFCallGraphDisable.getNumOccurrences() > 0) ? true : false;
  auto disableAllocAA =
      (PDGAllocAADisable.getNumOccurrences() > 0) ? true : false;
  auto disableRA = (PDGRADisable.getNumOccurrences() > 0) ? true : false;

  /*
   * Allocate the managers.
   */
  om = new CompilationOptionsManager(
      M,
      optMaxCores,
      (ND_PRVGs.getNumOccurrences() > 0),
      (DisableFloatAsReal.getNumOccurrences() == 0),
      (InlinerDisableHoistToMain.getNumOccurrences() > 0));

  /*
   * Fetch the other passes.
   */
  auto getSCEV = [this](Function &F) -> llvm::ScalarEvolution & {
    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(F).getSE();
    return SE;
  };
  auto getLoopInfo = [this](Function &F) -> llvm::LoopInfo & {
    auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    return LI;
  };
  auto getPDT = [this](Function &F) -> llvm::PostDominatorTree & {
    auto &PDT = getAnalysis<PostDominatorTreeWrapperPass>(F).getPostDomTree();
    return PDT;
  };
  auto getDT = [this](Function &F) -> llvm::DominatorTree & {
    auto &DT = getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    return DT;
  };
  auto getAssumptionCache = [this](Function &F) -> llvm::AssumptionCache & {
    auto &c = getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
    return c;
  };
  auto getCallGraph = [this](void) -> llvm::CallGraph & {
    auto &cg = getAnalysis<CallGraphWrapperPass>().getCallGraph();
    return cg;
  };
  auto getAA = [this](Function &F) -> llvm::AAResults & {
    auto &aa = getAnalysis<AAResultsWrapperPass>(F).getAAResults();
    return aa;
  };
  auto getBFI = [this](Function &F) -> llvm::BlockFrequencyInfo & {
    auto &b = getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
    return b;
  };
  auto getBPI = [this](Function &F) -> llvm::BranchProbabilityInfo & {
    auto &b = getAnalysis<BranchProbabilityInfoWrapperPass>(F).getBPI();
    return b;
  };

  /*
   * Allocate NOELLE.
   */
  this->n = new Noelle(M,
                       getSCEV,
                       getLoopInfo,
                       getPDT,
                       getDT,
                       getAssumptionCache,
                       getCallGraph,
                       getAA,
                       getBFI,
                       getBPI,
                       enabledTransformations,
                       verbose,
                       pdgVerbose,
                       minHot,
                       ldgGenerator,
                       om,
                       dumpPDG,
                       performThePDGComparison,
                       disableSVF,
                       disableSVFCallGraph,
                       disableAllocAA,
                       disableRA);

  return false;
}

Noelle &NoellePass::getNoelle(void) const {
  return *(this->n);
}

// Next there is code to register your pass to "opt"
char NoellePass::ID = 0;
static RegisterPass<NoellePass> X("noelle", "The NOELLE framework");

} // namespace arcana::noelle
