#include "arcana/noelle/core/NoellePass.hpp"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/CFLSteensAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/Analysis/ObjCARCAliasAnalysis.h"

using namespace arcana::noelle;

namespace {

struct CAT : public PassInfoMixin<CAT> {

  PreservedAnalyses run(Module &M, llvm::ModuleAnalysisManager &AM) {

    /*
     * Fetch NOELLE
     */
    auto &noelle = AM.getResult<NoellePass>(M);

    /*
     * Fetch the PDG
     */
    auto PDG = noelle.getProgramDependenceGraph();

    /*
     * Fetch the FDG of "main"
     */
    auto fm = noelle.getFunctionsManager();
    auto mainF = fm->getEntryFunction();
    auto FDG = PDG->createFunctionSubgraph(*mainF);

    /*
     * Iterate over the dependences
     */
    auto iterF = [](Value *src, DGEdge<Value, Value> *dep) -> bool {
      errs() << "   " << *src << " ";

      if (isa<ControlDependence<Value, Value>>(dep)) {
        errs() << " CONTROL ";
      } else {
        errs() << " DATA ";
        auto dataDep = cast<DataDependence<Value, Value>>(dep);
        if (dataDep->isRAWDependence()) {
          errs() << " RAW ";
        }
        if (dataDep->isWARDependence()) {
          errs() << " WAR ";
        }
        if (dataDep->isWAWDependence()) {
          errs() << " WAW ";
        }
        if (isa<MemoryDependence<Value, Value>>(dataDep)) {
          errs() << " MEMORY ";
          auto memDep = cast<MemoryDependence<Value, Value>>(dataDep);
          if (isa<MustMemoryDependence<Value, Value>>(memDep)) {
            errs() << " MUST ";
          } else {
            errs() << " MAY ";
          }
        }
      }

      errs() << "\n";
      return false;
    };

    for (auto &inst : instructions(mainF)) {
      errs() << "Instruction \"" << inst << "\" depends on\n";
      FDG->iterateOverDependencesTo(&inst, true, true, true, iterF);
    }

    for (auto &inst : instructions(mainF)) {
      errs() << "Instruction \"" << inst << "\" outgoing dependences\n";
      FDG->iterateOverDependencesFrom(&inst, true, true, true, iterF);
    }

    for (auto &inst : instructions(mainF)) {
      for (auto &inst2 : instructions(mainF)) {
        for (auto dep : FDG->getDependences(&inst, &inst2)) {
        }
      }
    }
    errs() << "A DGEdge  = " << sizeof(DGEdge<Value, Value>) << "\n";
    errs()
        << "A Control  = " << sizeof(ControlDependence<Value, Value>) << "\n";
    errs()
        << "A Variable  = " << sizeof(VariableDependence<Value, Value>) << "\n";
    errs() << "A Memory  = " << sizeof(MemoryDependence<Value, Value>) << "\n";

    return PreservedAnalyses::all();
  }
};

// Next there is code to register your pass to "opt"
AAManager NOELLE_createAliasAnalysesPipeline(void) {
  AAManager AA;

  AA.registerFunctionAnalysis<TypeBasedAA>();
  AA.registerModuleAnalysis<GlobalsAA>();
  AA.registerFunctionAnalysis<ScopedNoAliasAA>();
  AA.registerFunctionAnalysis<SCEVAA>();
  AA.registerFunctionAnalysis<CFLSteensAA>();
  AA.registerFunctionAnalysis<CFLAndersAA>();
  AA.registerFunctionAnalysis<llvm::objcarc::ObjCARCAA>();

  return AA;
}

llvm::PassPluginLibraryInfo getPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "CAT",
    LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      /*
       * REGISTRATION FOR "opt -passes='PDGEmbedder'"
       *
       */
      PB.registerPipelineParsingCallback(
          [](StringRef Name,
             llvm::ModulePassManager &PM,
             ArrayRef<llvm::PassBuilder::PipelineElement>) {
            if (Name == "CAT") {
              PM.addPass(CAT());
              return true;
            }
            return false;
          });

      /*
       * REGISTRATION FOR "AM.getResult<NoellePass>()"
       */
      PB.registerAnalysisRegistrationCallback([](FunctionAnalysisManager &AM) {
        AM.registerPass([&] { return NOELLE_createAliasAnalysesPipeline(); });
      });

      PB.registerAnalysisRegistrationCallback([](ModuleAnalysisManager &AM) {
        AM.registerPass([&] { return NoellePass(); });
      });
    }
  };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getPluginInfo();
}

} // namespace
