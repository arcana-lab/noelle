

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

#include <UniqueIRMarkerPass.hpp>
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
  auto AAResultBaseResult = AAResultBase::alias(LocA, LocB);


  if (auto *InsA = dyn_cast<Instruction>(LocA.Ptr), *InsB = dyn_cast<Instruction>(LocB.Ptr);
      InsA && InsB) {
    auto &Wrapper = MP.getAnalysis<AAResultsWrapperPass>(*const_cast<Instruction *>(InsA)->getFunction());
    auto aliasRes = Wrapper.getAAResults().alias(LocA, LocB);
    auto InsPair = std::make_pair(LocA.Ptr, LocB.Ptr);
    auto Match = [&InsPair](OracleAliasFunctionResults::Dependencies DependencySet, OracleAliasFunctionResults::DependencyType DType) -> optional<AliasResult> {
      if (std::find(DependencySet.begin(), DependencySet.end(), InsPair) != DependencySet.end()) {
        return MustAlias;
      }
      return nullopt;
    };

    auto NoMatch = [AAResultBaseResult]() { return AAResultBaseResult; };

    auto OracleResult = SearchResult<AliasResult>(InsA, InsB, Match, NoAlias, NoMatch);
    if (OracleResult == NoAlias) {
      assert ((AAResultBaseResult == NoAlias || AAResultBaseResult == MayAlias)
      && "If Oracle find NoAlias then AAResults must find No or MayAlias");

      assert ((aliasRes == NoAlias || aliasRes == MayAlias)
                  && "If Oracle find NoAlias then AAResultsWrapper must find No or MayAlias");
    }
    if (OracleResult == MustAlias) {
      assert ((AAResultBaseResult == MustAlias || AAResultBaseResult == MayAlias || AAResultBaseResult == PartialAlias)
      && "If Oracle finds MustAlias then AAResults must find Must, Partial or MayAlias");

      assert ((aliasRes == MustAlias || aliasRes == MayAlias || aliasRes == PartialAlias)
                  && "If Oracle finds MustAlias then AAResultsWrapper must find Must, Partial or MayAlias");

    }
    return OracleResult;
  }

  return AAResultBaseResult;
}

Loop *OracleDDGAAResult::getTopMostLoop(Loop *La) const {
  while (La->getLoopDepth() > 1) {
          La = La->getParentLoop();
  }
  return La;
}

ModRefInfo OracleDDGAAResult::getModRefInfo(const ImmutableCallSite CS, const MemoryLocation &Loc) {
  auto ModRefInfoBase = AAResultBase::getModRefInfo(CS, Loc);

  if (auto *I = dyn_cast<Instruction>(Loc.Ptr); I) {
    auto Match = [CS, I](OracleAliasFunctionResults::Dependencies DependencySet, OracleAliasFunctionResults::DependencyType DType) -> optional<ModRefInfo> {
      auto equalEither = [CS, I] (OracleAliasFunctionResults::Dependency d) {
        return (CS.getInstruction() == d.first && I == d.second) || (CS.getInstruction() == d.second && I == d.first);
      };
      if (std::find_if(DependencySet.begin(), DependencySet.end(), equalEither) != DependencySet.end()) {
        return MRI_ModRef;
      }
      return nullopt;
    };

    auto NoMatch = [&CS, &Loc, this]() -> ModRefInfo { AAResultBase::getModRefInfo(CS, Loc); };

    auto OracleModRefInfo = SearchResult<ModRefInfo>(CS.getInstruction(), I, Match, MRI_NoModRef, NoMatch);
    errs() << "CS: "; CS->print(errs()); errs() << '\n';
    errs() << "Loc: "; Loc.Ptr->print(errs()); errs() << '\n';
    errs() << "OracleModRefInfo: " << OracleModRefInfo << " BaseModRefInfo: " << ModRefInfoBase << '\n';
    if (OracleModRefInfo == MRI_NoModRef) {
      assert ((ModRefInfoBase == MRI_NoModRef)
                  && "If Oracle find MRI_NoModRef then AAResultBase must find NoModRef");
    }
    if (OracleModRefInfo == MRI_ModRef) {
      assert ((ModRefInfoBase == MRI_ModRef || ModRefInfoBase == MRI_Mod || ModRefInfoBase == MRI_Ref)
                  && "If Oracle finds MustAlias then AAResultBase must find ModRef, Mod or Ref");

    }
    return OracleModRefInfo;
  }

  return AAResultBase::getModRefInfo(CS, Loc);
}

ModRefInfo OracleDDGAAResult::getModRefInfo(ImmutableCallSite CS1, ImmutableCallSite CS2) {
  return AAResultBase::getModRefInfo(CS1, CS2);
}

template <typename V>
V OracleDDGAAResult::SearchResult(const Instruction *InsA, const Instruction *InsB,
                 std::function<optional<V>(OracleAliasFunctionResults::Dependencies, OracleAliasFunctionResults::DependencyType)> Match, V None, std::function<V(void)> NoMatch) {

  auto IAMID = UniqueIRMarkerReader::getModuleID(InsA->getModule());
  auto IBMID = UniqueIRMarkerReader::getModuleID(InsB->getModule());
  auto IAFID = UniqueIRMarkerReader::getFunctionID(InsA->getFunction());
  auto IBFID = UniqueIRMarkerReader::getFunctionID(InsB->getFunction());

  if (IAMID && IAMID == IBMID && IAFID && IAFID == IBFID) {

    auto &LI = MP.getAnalysis<LoopInfoWrapperPass>(*const_cast<Instruction *>(InsA)->getFunction()).getLoopInfo();
    auto IALoop = LI.getLoopFor(InsA->getParent());
    auto IBLoop = LI.getLoopFor(InsB->getParent());
    if (IALoop && IBLoop) {
      IALoop = getTopMostLoop(IALoop);
      IBLoop = getTopMostLoop(IBLoop);
      if (auto IALoopID = UniqueIRMarkerReader::getLoopID(IALoop); IALoop == IBLoop && IALoopID) {

        for (auto &[res, type] : Res->getFunctionResults(IAMID.value(),
                                                         IAFID.value(),
                                                         IALoopID.value()).dependencies) {
          auto result = Match(res, type);
          if (result) {
            return result.value();
          }
        }
        return None;
      }
    }
  }
  return NoMatch();
}

void OracleAAWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<UniqueIRMarkerPass>(); // Must mark up the IR
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
  M.print(errs(), nullptr);
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




