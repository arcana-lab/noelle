#include <cstdint>
#include <string>

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"

#include "arcana/noelle/core/Noelle.hpp"
#include "arcana/noelle/core/NoellePass.hpp"

#include "LDGDot.hpp"
#include "Pass.hpp"

using namespace std;
using namespace llvm;

namespace arcana::noelle {

/*
 * Options should starts with the same unique prefix (e.g. "ldg-dot-"), so that
 * they can be grep'd from `opt --help with not collisions`. If other prefixes
 * are used, the tool `noelle-ldg-dot` should be modified accordingly
 */
static cl::opt<uint64_t> OptLoopId("ldg-dot-loop-id",
                                   cl::desc("Target loop ID"),
                                   cl::Required);
static cl::opt<string> OptOutputFile("ldg-dot-output-file",
                                     cl::desc("Output file for the dot graph"));
static cl::opt<bool> OptCollapseEdges(
    "ldg-dot-collapse-edges",
    cl::desc("Edges from and to the same node will be collapsed into one"));
static cl::opt<bool> OptOnlyLCEdges(
    "ldg-dot-only-lc-edges",
    cl::desc("Show only loop-carried dependences"));
static cl::opt<bool> OptCFG("ldg-dot-cfg",
                            cl::desc("Show control dependences"));
static cl::opt<bool> OptHideKnownSCCs(
    "ldg-dot-hide-known-sccs",
    cl::desc("Hide SCC that are not LoopCarriedUnknownSCC"));

DotPass::DotPass() : ModulePass{ ID }, log(NoelleLumberjack, "LDGDot") {}

bool DotPass::doInitialization(Module &M) {
  return false;
}

void DotPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<NoellePass>();
}

bool DotPass::runOnModule(Module &M) {
  auto &noelle = getAnalysis<NoellePass>().getNoelle();
  bool found = false;

  if (OptLoopId.getNumOccurrences() > 0) {
    auto &LSs = *noelle.getLoopStructures();
    for (auto LS : LSs) {
      if (LS->getID().value() == OptLoopId) {
        runOnLoop(noelle, LS);
        found = true;
        break;
      }
    }
  }

  if (!found) {
    log.bypass() << "ERROR: target loop not found\n";
  }

  return false;
}

void DotPass::runOnLoop(Noelle &noelle, LoopStructure *LS) {
  string outputFile;
  if (OptOutputFile.getNumOccurrences() == 0) {
    outputFile = "ldg_id_" + to_string(OptLoopId) + ".dot";
  } else {
    outputFile = OptOutputFile;
  }

  DotOptions options = 0;
  if (OptCollapseEdges) {
    options |= COLLAPSE_EDGES;
  }
  if (OptOnlyLCEdges) {
    options |= ONLY_LC_EDGES;
  }
  if (OptHideKnownSCCs) {
    options |= HIDE_KNOWN_SCCS;
  }

  auto optimizations = { LoopContentOptimization::MEMORY_CLONING_ID,
                         LoopContentOptimization::THREAD_SAFE_LIBRARY_ID };
  auto LC = noelle.getLoopContent(LS, optimizations);

  exportToDotGraph(LC, outputFile, options);

  log.bypass() << "Dot file written to " << outputFile << "\n";
}

char DotPass::ID = 0;
static RegisterPass<DotPass> X("LDGDot",
                               "Dumps loop SCCDAGs into Dot file",
                               false,
                               false);

} // namespace arcana::noelle
