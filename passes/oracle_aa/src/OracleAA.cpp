

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
#include "VIAInvoker.hpp"
#include "OracleAA.hpp"

using namespace llvm;

cl::list<std::string> Inputs("input-args", cl::desc("List of arguments to pass to the instrumented binary for execution"
                                                     " having more than one least to multiple executions where "
                                                     "the results are merged"),
                                               cl::value_desc("arguments"), cl::OneOrMore);

OracleDDGAAResult::OracleDDGAAResult() : Res(std::make_unique<OracleAliasResults>()), AAResultBase() {
  errs() << "ctr AAResult\n";
}

AliasResult OracleDDGAAResult::query(const MemoryLocation &, const MemoryLocation &) {
  errs() << "checking aliasA";
  return MustAlias;
}
AliasResult OracleDDGAAResult::alias(const MemoryLocation &LocA, const MemoryLocation &LocB) {
  errs() << "alias query\n";
  auto InsA = dyn_cast<Instruction>(LocA.Ptr);
  auto InsB = dyn_cast<Instruction>(LocB.Ptr);
  if (!InsA || !InsB) {
    errs() << "Cannot handle "; LocA.Ptr->print(errs()); errs() << " "; LocB.Ptr->print(errs()); errs() << '\n';
    // TODO: Handle this
    return MayAlias;
//    assert( 0 && "Cannot continue" );
  }

  auto Fa = UniqueIRMarkerReader::getFunctionID(InsA->getFunction());
  auto Fb = UniqueIRMarkerReader::getFunctionID(InsB->getFunction());
  auto Ma = UniqueIRMarkerReader::getModuleID(InsA->getModule());
  auto Mb = UniqueIRMarkerReader::getModuleID(InsB->getModule());

  assert (Ma == Mb && Fa == Fb && "Not same module and function");
  auto &res = Res->getFunctionResults(Ma, Fa);
  auto search = std::pair<const Value *, const Value *>( LocA.Ptr, LocB.Ptr );


  if ( res.ReadAfterWrite.find(search) != res.ReadAfterWrite.end() ) {
    errs() << "RaW";
    return MustAlias;
  } else if ( res.ReadAfterRead.find(search) != res.ReadAfterRead.end() ) {
    errs() << "RaR";
    return NoAlias;
  } else if ( res.WriteAfterRead.find(search) != res.WriteAfterRead.end() ) {
    errs() << "WaR";
    return MustAlias;
  } else if ( res.WriteAfterWrite.find(search) != res.WriteAfterWrite.end() ) {
    errs() << "WaW";
    return MustAlias;
  } else {
//    errs() << "Not seen InstA: "; LocA.Ptr->print(errs()); errs() << " |or B: ";
//    LocB.Ptr->print(errs()); errs() << '\n';
    return AAResultBase::alias(LocA, LocB);
  }
}

void OracleAAWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<UniqueIRMarkerPass>(); // Must mark up the IR
  AU.setPreservesAll();
}

bool llvm::OracleAAWrapperPass::doInitialization(llvm::Module &M) {

  errs() << "do Init AAWrapper\n";
  Result = std::make_unique<OracleDDGAAResult>();
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


char OracleAAWrapperPass::ID = 0;
static RegisterPass<OracleAAWrapperPass> X("OracleAA", "Instruments and executes the current IR to find memory "
                                                          "dependencies at runtime for a given input");


//static llvm::legacy::RegisterStandardPasses
//    RegisterMyPass(PassManagerBuilder::EP_ModuleOptimizerEarly, registerSkeletonPass);
//
//static RegisterStandardPasses
//    RegisterMyPass0(PassManagerBuilder::EP_EnabledOnOptLevel0, registerSkeletonPass);




