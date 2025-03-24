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

class LDGDotPass : public llvm::PassInfoMixin<LDGDotPass> {
public:
  LDGDotPass() : log(NoelleLumberjack, "LDGDot") {}

  void runOnLoop(Noelle &noelle, LoopStructure *LS) {
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

  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &MAM) {
    auto &noelle = MAM.getResult<NoellePass>(M);
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

    return llvm::PreservedAnalyses::all();
  }

private:
  noelle::Logger log;
};

// Next there is code to register your pass to "opt"
llvm::PassPluginLibraryInfo getPluginInfo() {
  return { LLVM_PLUGIN_API_VERSION,
           "noelle-ldg-dot",
           LLVM_VERSION_STRING,
           [](PassBuilder &PB) {
             /*
              * REGISTRATION FOR "opt -passes='noelle-rm-function'"
              *
              */
             PB.registerPipelineParsingCallback(
                 [](StringRef Name,
                    llvm::ModulePassManager &PM,
                    ArrayRef<llvm::PassBuilder::PipelineElement>) {
                   if (Name == "noelle-ldg-dot") {
                     PM.addPass(LDGDotPass());
                     return true;
                   }
                   return false;
                 });

             /*
              * REGISTRATION FOR "AM.getResult<NoellePass>()"
              */
             PB.registerAnalysisRegistrationCallback(
                 [](ModuleAnalysisManager &AM) {
                   AM.registerPass([&] { return NoellePass(); });
                 });
           } };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getPluginInfo();
}

} // namespace arcana::noelle
