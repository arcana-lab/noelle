

#include <memory>
#include <set>

#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/DominanceFrontier.h>
#include <llvm/Analysis/ScalarEvolutionAliasAnalysis.h>
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/ValueTracking.h"
#include <llvm/Analysis/MemorySSA.h>
#include <llvm/Analysis/AliasSetTracker.h>
#include <llvm/Analysis/RegionInfo.h>
#include <llvm/Support/raw_ostream.h>

#include <UniqueIRMarker.hpp>
#include <UniqueIRMarkerReader.hpp>
#include <OracleAA.hpp>

#include "VIAInvoker.hpp"
#include "OracleAA.hpp"

using namespace llvm;

cl::list<std::string> Inputs("input-args", cl::desc("List of arguments to pass to the instrumented binary for execution"
                                                     " having more than one least to multiple executions where "
                                                     "the results are merged"),
                                               cl::value_desc("arguments"), cl::OneOrMore);

OracleDDGAAResult::OracleDDGAAResult(ModulePass &MP) : MP(MP), Res(std::make_unique<OracleAliasResults>()), AAResultBase() {
}

AliasResult OracleDDGAAResult::query(const MemoryLocation &, const MemoryLocation &) {
  return MustAlias;
}
AliasResult OracleDDGAAResult::alias(const MemoryLocation &LocA, const MemoryLocation &LocB) {
  errs() << "alias query\n";
  auto InsA = dyn_cast<Instruction>(LocA.Ptr);
  auto InsB = dyn_cast<Instruction>(LocB.Ptr);

  if (InsA && InsB) {
    auto *Fa = InsA->getFunction();
    auto *Fb = InsB->getFunction();
    if (Fa == Fb) {
      auto FaID = UniqueIRMarkerReader::getFunctionID(Fa);
      auto Ma = UniqueIRMarkerReader::getModuleID(InsA->getModule());
      if (!Fa->empty()) {
        auto &FaRef = const_cast<Function &>(*Fa);
        auto &LIa = MP.getAnalysis<LoopInfoWrapperPass>(FaRef).getLoopInfo();
        auto *La = LIa.getLoopFor(InsA->getParent());
        auto *Lb = LIa.getLoopFor(InsB->getParent());
        if (La && Lb) {
          La = getTopMostLoop(La);
          Lb = getTopMostLoop(Lb);
          auto LaID = UniqueIRMarkerReader::getLoopID(La);
          if (La == Lb && LaID) {
            auto search = std::pair<const Value *, const Value *>(LocA.Ptr, LocB.Ptr);
            for (auto &res : Res->getFunctionResults(Ma.value(), FaID.value(), LaID.value()).dependencies) {
              if (res.find(search) != res.end()) {
                return MustAlias;
              }
            }
            // Since LocA and LocB are in the same loop and there is not recorded alias we know they don't alias.
            return NoAlias;
          }
        }
      }
    }
  }
  // If the Locations are from different loops then this pass doesn't know if these Locations alias
  return AAResultBase::alias(LocA, LocB);
}

Loop *OracleDDGAAResult::getTopMostLoop(Loop *La) const {
  while (La->getLoopDepth() > 1) {
          La = La->getParentLoop();
  }
  return La;
}

ModRefInfo OracleDDGAAResult::getModRefInfo(const ImmutableCallSite CS, const MemoryLocation &Loc) {
  if (auto *I = dyn_cast<Instruction>(Loc.Ptr); I) {
    auto IMID = UniqueIRMarkerReader::getModuleID(I->getModule());
    auto IFID = UniqueIRMarkerReader::getFunctionID(I->getFunction());
    auto CSMID = UniqueIRMarkerReader::getModuleID(CS.getCalledFunction()->getParent());
    auto CSFID = UniqueIRMarkerReader::getFunctionID(CS.getCalledFunction());
    if (IMID && IMID == CSMID && IFID && IFID == CSFID) {
      auto &LI = MP.getAnalysis<LoopInfoWrapperPass>(*const_cast<Instruction *>(I)->getFunction()).getLoopInfo();
      auto IL = LI.getLoopFor(I->getParent());
      auto CSL = LI.getLoopFor(CS->getParent());
      if (IL && CSL) {
        IL = getTopMostLoop(IL);
        CSL = getTopMostLoop(CSL);
        if (auto ILID = UniqueIRMarkerReader::getLoopID(IL); IL == CSL && ILID) {
          auto Found = false;
          for ( auto &res : Res->getFunctionResults( IMID.value(), IFID.value(), ILID.value() ).dependencies ) {
            auto equalEither = [I] (OracleAliasFunctionResults::Dependency dep)
              { return dep.first == I || dep.second == I; };
            Found |= std::find_if(res.begin(), res.end(), equalEither) != res.end();
          }
          if (Found) {
            return ModRefInfo::MRI_ModRef;
          } else {
            return ModRefInfo::MRI_NoModRef;
          }
        }
      }
    }
  }

  return AAResultBase::getModRefInfo(CS, Loc);
}

ModRefInfo OracleDDGAAResult::getModRefInfo(ImmutableCallSite CS1, ImmutableCallSite CS2) {
  return AAResultBase::getModRefInfo(CS1, CS2);
}

//ModRefInfo OracleDDGAAResult::getModRefInfo(const CallInst *C, const MemoryLocation &Loc) {
//  auto callID = UniqueIRMarkerReader::getInstructionID(C);
//  return MRI_Ref;
//}

void OracleAAWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<UniqueIRMarkerPass>(); // Must mark up the IR
  AU.addRequired<AAResultsWrapperPass>();
  AU.setPreservesAll();
}

bool llvm::OracleAAWrapperPass::doInitialization(llvm::Module &M) {
  Result = std::make_unique<OracleDDGAAResult>(*this);
  return true;
}
bool OracleAAWrapperPass::runOnModule(Module &M) {
  oracle_aa::VIAInvoker viaInvoker(M, *this);
  assert(!Inputs.empty() && "Need at least one input argument");
  viaInvoker.runInference( Inputs.front() );
  auto res = viaInvoker.getResults();
  Result->getAliasResults()->unionFunctionAlias( *res );
  return true;
}

ModulePass* llvm::createOracleDDGAAWrapperPass() {
  return new OracleAAWrapperPass();
}

const OracleDDGAAResult &OracleAAWrapperPass::getResult() const {
  return *Result;
}

OracleDDGAAResult &OracleAAWrapperPass::getResult() {
    return const_cast<OracleDDGAAResult &>(const_cast<const OracleAAWrapperPass *>(this)->getResult());
}


char OracleAAWrapperPass::ID = 0;
static RegisterPass<OracleAAWrapperPass> X("OracleAA", "Instruments and executes the current IR to find memory "
                                                          "dependencies at runtime for a given input");


//static llvm::legacy::RegisterStandardPasses
//    RegisterMyPass(PassManagerBuilder::EP_ModuleOptimizerEarly, registerSkeletonPass);
//
//static RegisterStandardPasses
//    RegisterMyPass0(PassManagerBuilder::EP_EnabledOnOptLevel0, registerSkeletonPass);




