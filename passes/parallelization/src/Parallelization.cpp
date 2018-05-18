#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"

#include "Parallelization.hpp"

using namespace llvm;

bool llvm::Parallelization::doInitialization (Module &M){
  errs() << "Parallelization at \"doInitialization\"\n" ;
  return false;
}

void llvm::Parallelization::getAnalysisUsage(AnalysisUsage &AU) const {

  return ;
}

bool llvm::Parallelization::runOnModule (Module &M){
  errs() << "Parallelization at \"runOnModule\"\n" ;

  return false;
}

llvm::Parallelization::Parallelization() : ModulePass{ID}{
  return ;
}

llvm::Function * llvm::Parallelization::createFunctionForTheLoopBody (){

}

llvm::Parallelization::~Parallelization(){
  return ;
}

// Next there is code to register your pass to "opt"
char llvm::Parallelization::ID = 0;
static RegisterPass<Parallelization> X("parallelization", "Computing the Program Dependence Graph");

// Next there is code to register your pass to "clang"
static Parallelization * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new Parallelization());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new Parallelization());}});// ** for -O0
