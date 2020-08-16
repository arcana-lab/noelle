/*
 * Copyright 2016 - 2020  Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopStats.hpp"

#include "PDGPrinter.hpp"

using namespace llvm;

bool LoopStats::doInitialization(Module &M) {
  return false;
}

void LoopStats::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<Noelle>();
  return;
}

bool LoopStats::runOnModule(Module &M) {
  auto& noelle = getAnalysis<Noelle>();

  if (noelle.getVerbosity() > Verbosity::Disabled) {
    errs() << "LoopStats: Start\n";
  }

  auto programLoops = noelle.getLoops();
  for (auto loop : *programLoops) {
    if (noelle.getVerbosity() > Verbosity::Disabled) {
      errs() << "LoopStats: Collecting stats for: \n";
    }

    collectStatsForLoop(*loop);
  }

  /*
   * TODO: Construct dependence graph using only LLVM APIs
   */
  for (auto LDI : *programLoops) {
    auto loopStructure = LDI->getLoopStructure();
    auto loopHeader = loopStructure->getHeader();
    auto loopFunction = loopStructure->getFunction();

    auto id = loopStructure->getID();
    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*loopFunction).getSE();
    auto &LI = getAnalysis<LoopInfoWrapperPass>(*loopFunction).getLoopInfo();
    auto llvmLoop = LI.getLoopFor(loopHeader);
    auto loopDG = LDI->getLoopDG();

    collectStatsForLoop(id, SE, loopDG, *llvmLoop);
  }

  printStatsHumanReadable();

  delete programLoops ;

  if (noelle.getVerbosity() > Verbosity::Disabled) {
    errs() << "LoopStats: Exit\n";
  }

  return false;
}

void LoopStats::collectStatsForLoop (int id, ScalarEvolution &SE, PDG *loopDG, Loop &llvmLoop) {
  Stats *statsForLoop = new Stats();
  statsByLoopAccordingToLLVM.insert(std::make_pair(id, statsForLoop));
  statsForLoop->loopID = id;

  auto loopFunction = llvmLoop.getHeader()->getParent();

  collectStatsOnLLVMIVs(SE, llvmLoop, statsForLoop);
  collectStatsOnLLVMInvariants(llvmLoop, statsForLoop);
  collectStatsOnLLVMSCCs(loopDG, statsForLoop);
}

void LoopStats::collectStatsOnLLVMIVs (ScalarEvolution &SE, Loop &llvmLoop, Stats *statsForLoop) {
  for (auto &phi : llvmLoop.getHeader()->phis()) {
    if (llvmLoop.isAuxiliaryInductionVariable(phi, SE)) {
      statsForLoop->numberOfIVs++;
    }
  }

  auto governingIV = llvmLoop.getInductionVariable(SE);
  statsForLoop->isGovernedByIV = governingIV != nullptr;

  /*
   * LLVM does not provide a way to collect all instructions used in computing IVs
   */
  statsForLoop->numberOfComputingInstructionsForIVs = -1;
}

void LoopStats::collectStatsOnLLVMInvariants (Loop &llvmLoop, Stats *statsForLoop) {
  for (auto &B : llvmLoop.getBlocks()) {
    for (auto &I : *B) {
      if (!llvmLoop.isLoopInvariant(&I)) continue;
      statsForLoop->numberOfInvariantsContainedWithinTheLoop++;
    }
  }
}

void LoopStats::collectStatsOnLLVMSCCs (PDG *loopDG, Stats *statsForLoop) {

  /*
   * Construct loop internal SCCDAG (it uses LLVM's scc_iterator)
   */
  std::vector<Value *> loopInternals;
  for (auto internalNode : loopDG->internalNodePairs()) {
    loopInternals.push_back(internalNode.first);
  }
  auto loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
  auto loopInternalSCCDAG = SCCDAG(loopInternalDG);
  collectStatsOnSCCDAG(&loopInternalSCCDAG, nullptr, statsForLoop);
}

void LoopStats::collectStatsForLoop (LoopDependenceInfo &LDI) {

  auto loopStructure = LDI.getLoopStructure();
  Stats *statsForLoop = new Stats();
  statsByLoopAccordingToNoelle.insert(std::make_pair(LDI.getID(), statsForLoop));
  statsForLoop->loopID = loopStructure->getID();

  collectStatsOnNoelleIVs(LDI, statsForLoop);
  collectStatsOnNoelleSCCs(LDI, statsForLoop);
  collectStatsOnNoelleInvariants(LDI, statsForLoop);
}

void LoopStats::collectStatsOnNoelleInvariants (LoopDependenceInfo &LDI, Stats *stats) {
  auto invariantManager = LDI.getInvariantManager();
  stats->numberOfInvariantsContainedWithinTheLoop =
    invariantManager->getLoopInstructionsThatAreLoopInvariants().size();
}

void LoopStats::collectStatsOnNoelleIVs (LoopDependenceInfo &LDI, Stats *statsForLoop) {
  auto loopStructure = LDI.getLoopStructure();
  auto ivManager = LDI.getInductionVariableManager();
  auto ivs = ivManager->getInductionVariables(*loopStructure);

  statsForLoop->numberOfIVs = ivs.size();
  for (auto iv : ivs) {
    statsForLoop->numberOfComputingInstructionsForIVs += iv->getAllInstructions().size();
  }

  statsForLoop->isGovernedByIV = ivManager->getLoopGoverningIVAttribution(*loopStructure) != nullptr;
}

void LoopStats::collectStatsOnNoelleSCCs (LoopDependenceInfo &LDI, Stats *statsForLoop) {

  /*
   * HACK: we need to re-compute SCCDAGAttrs instead of using the one provided by LDI
   * because we do NOT want to merge SCC as a convenience to parallelization schemes
   *
   * Once this hack is removed, this can go away
   */
  auto loopStructure = LDI.getLoopStructure();
  auto loopDG = LDI.getLoopDG();
  std::vector<Value *> loopInternals;
  for (auto internalNode : loopDG->internalNodePairs()) {
    loopInternals.push_back(internalNode.first);
  }
  auto loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
  auto loopInternalSCCDAG = SCCDAG(loopInternalDG);

  auto &loopHierarchyConst = LDI.getLoopHierarchyStructures();
  auto &loopHierarchy = (LoopsSummary&)loopHierarchyConst;
  auto loopFunction = loopStructure->getFunction();
  DominatorTree DT(*loopFunction);
  PostDominatorTree PDT(*loopFunction);
  DominatorSummary DS(DT, PDT);

  auto loopExitBlocks = loopStructure->getLoopExitBasicBlocks();
  auto environment = LoopEnvironment(loopDG, loopExitBlocks);
  auto invariantManager = LDI.getInvariantManager();
  LoopCarriedDependencies lcd(loopHierarchy, DS, loopInternalSCCDAG);
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*loopFunction).getSE();
  auto inductionVariables = InductionVariableManager(loopHierarchy, *invariantManager, SE, loopInternalSCCDAG, environment);
  auto sccdagAttrs = SCCDAGAttrs(loopDG, &loopInternalSCCDAG, loopHierarchy, SE, lcd, inductionVariables, DS);

  DGPrinter::writeGraph<SCCDAG, SCC>("sccdag-" + std::to_string(LDI.getID()) + ".dot", &loopInternalSCCDAG);
  collectStatsOnSCCDAG(&loopInternalSCCDAG, &sccdagAttrs, statsForLoop);
}

void LoopStats::collectStatsOnSCCDAG (SCCDAG *sccdag, SCCDAGAttrs *sccdagAttrs, Stats *statsForLoop) {

  /*
   * For every SCC object contained in an un-merged SCCDAG, we need to determine
   * whether it is a single independent instruction or a strongly connected component.
   * We do this by ensuring no edge within the SCC object is internal
   */
  for (auto node : sccdag->getNodes()) {
    auto scc = node->getT();

    statsForLoop->numberOfNodesInSCCDAG++;

    bool isSCC = false;
    for (auto edge : scc->getEdges()) {
      if (scc->isInternal(edge->getOutgoingT()) && scc->isInternal(edge->getIncomingT())) {
        isSCC = true;
        break;
      }
    }
    if (!isSCC) continue;

    statsForLoop->numberOfSCCs++;

    if (sccdagAttrs) {
      auto sccAttrs = sccdagAttrs->getSCCAttrs(scc);
      if (sccAttrs->getType() != SCCAttrs::SCCType::SEQUENTIAL) continue;
    }
    statsForLoop->numberOfSequentialSCCs++;
  }
}

void LoopStats::printStats (Stats *stats) {
  errs() << "Loop: " << stats->loopID << "\n";
  errs() << "Number of IVs: " << stats->numberOfIVs << "\n";
  errs() << "Number of instructions in those IVs: " << stats->numberOfComputingInstructionsForIVs << "\n";
  errs() << "Number of invariants contained within loop: " << stats->numberOfInvariantsContainedWithinTheLoop << "\n";
  errs() << "Has loop governing IV: " << stats->isGovernedByIV << "\n";
  errs() << "Number of nodes in SCCDAG: " << stats->numberOfNodesInSCCDAG << "\n";
  errs() << "Number of SCCs: " << stats->numberOfSCCs << "\n";
  errs() << "Number of sequential SCCs: " << stats->numberOfSequentialSCCs << "\n";
}

void LoopStats::printStatsHumanReadable (void) {
  for (auto idAndNoelleLoop : statsByLoopAccordingToNoelle) {
    auto id = idAndNoelleLoop.first;
    auto noelleStats = idAndNoelleLoop.second;
    auto llvmStats = statsByLoopAccordingToLLVM.at(id);
    errs() << "Noelle:\n";
    printStats(noelleStats);
    errs() << "LLVM:\n";
    printStats(llvmStats);
  }

  return;
}

LoopStats::LoopStats()
  : ModulePass{ID} {
  return;
}

LoopStats::~LoopStats() {

  for (auto loopAndStats : statsByLoopAccordingToLLVM) {
    delete loopAndStats.second;
  }
  for (auto loopAndStats : statsByLoopAccordingToNoelle) {
    delete loopAndStats.second;
  }
  statsByLoopAccordingToLLVM.clear();
  statsByLoopAccordingToNoelle.clear();

  return;
}

// Next there is code to register your pass to "opt"
char LoopStats::ID = 0;
static RegisterPass<LoopStats> X("LoopStats", "Generate statistics output for loops using LDI");

// Next there is code to register your pass to "clang"
static LoopStats * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LoopStats());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LoopStats()); }}); // ** for -O0
