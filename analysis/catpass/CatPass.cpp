#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/AliasAnalysis.h"

#include "../include/PDGAnalysis.hpp"

using namespace llvm;

/*
 * Alias information on each function used by the Program Dependence Graph
 */
struct FunctionAliasInfo {
  FunctionAliasInfo(AAResults *a) { aa = a; }

  AAResults* aa;
};

struct ModuleAliasInfo {
  std::map<Function *, FunctionAliasInfo *> aliasInfo;
};

bool llvm::PDGAnalysis::doInitialization (Module &M) {
  errs() << "PDGAnalysis at \"doInitialization\"\n" ;
  return false;
}

void llvm::PDGAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<AAResultsWrapperPass>();
  AU.setPreservesAll();
  return ;
}

void constructEdgesFromUseDefs (Module &M, std::unique_ptr<PDG> &pdg);
void constructEdgesFromAliases (Module &M, std::unique_ptr<PDG> &pdg, ModuleAliasInfo *aaInfo);

bool llvm::PDGAnalysis::runOnModule (Module &M) {
  errs() << "PDGAnalysis at \"runOnModule\"\n" ;
  this->programDependenceGraph = std::unique_ptr<PDG>(new PDG());
  auto *aaInfo = new ModuleAliasInfo();

  /*
   * Create AliasInfo for PDG use, then compute the PDG.
   */
  for (auto &F : M) {
    if (F.empty()) continue ;
    aaInfo->aliasInfo[&F] = new FunctionAliasInfo(&(getAnalysis<AAResultsWrapperPass>(F).getAAResults()));
  }

  this->programDependenceGraph->constructNodes(M);
  constructEdgesFromUseDefs(M, this->programDependenceGraph);
  constructEdgesFromAliases(M, this->programDependenceGraph, aaInfo);

  delete aaInfo;
  return false;
}

llvm::PDGAnalysis::PDGAnalysis() : ModulePass{ID}{
  return ;
}

llvm::PDG & llvm::PDGAnalysis::getPDG (){
  return *this->programDependenceGraph;
}

// Next there is code to register your pass to "opt"
char llvm::PDGAnalysis::ID = 0;
static RegisterPass<PDGAnalysis> X("PDGAnalysis", "Computing the Program Dependence Graph");

// Next there is code to register your pass to "clang"
static PDGAnalysis * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGAnalysis());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGAnalysis());}});// ** for -O0

void constructEdgesFromUseDefs (Module &M, std::unique_ptr<PDG> &pdg) {
  for (auto iNodePair : pdg->instructionNodePairs()) {
    Instruction *I = iNodePair.first;
    if (I->getNumUses() == 0)
      continue;
    for (auto& U : I->uses()) {
      auto user = U.getUser();
      if (auto userInst = dyn_cast<Instruction>(user)) {
        pdg->addEdgeFromTo(I, userInst);
      }
    }
  }

  return ;
}

void constructEdgesFromAliases (Module &M, std::unique_ptr<PDG> &pdg, ModuleAliasInfo *aaInfo) {
  // TODO:
  /*
   * Use alias information on stores and loads to construct edges between pairs of these instructions
   */
}
