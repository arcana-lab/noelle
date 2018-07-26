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

#include "llvm/ADT/iterator_range.h"

#include "../include/HeuristicsPass.hpp"
#include "PDGAnalysis.hpp"

using namespace llvm;

using namespace llvm;

bool llvm::HeuristicsPass::doInitialization (Module &M){
  return false;
}

void llvm::HeuristicsPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<PDGAnalysis>();
  AU.setPreservesAll();
  return ;
}

bool llvm::HeuristicsPass::runOnModule (Module &M){
  return false;
}

llvm::HeuristicsPass::HeuristicsPass() : ModulePass{ID}{
  return ;
}

llvm::Heuristics * llvm::HeuristicsPass::getHeuristics (){
  return new Heuristics();
}

// Next there is code to register your pass to "opt"
char llvm::HeuristicsPass::ID = 0;
static RegisterPass<HeuristicsPass> X("heuristics", "Heuristics about code");

// Next there is code to register your pass to "clang"
static HeuristicsPass * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new HeuristicsPass());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new HeuristicsPass());}});// ** for -O0
