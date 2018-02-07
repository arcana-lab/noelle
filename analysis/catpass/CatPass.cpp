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

bool llvm::PDGAnalysis::doInitialization (Module &M){
  errs() << "PDGAnalysis at \"doInitialization\"\n" ;
  return false;
}

void llvm::PDGAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<AAResultsWrapperPass>();
  AU.setPreservesAll();
  return ;
}

bool llvm::PDGAnalysis::runOnModule (Module &M){
  errs() << "PDGAnalysis at \"runOnModule\"\n" ;
  this->programDependenceGraph = std::unique_ptr<PDG>(new PDG());
  for (auto &F : M) {
    if (F.empty()) continue ;
    this->aaResults[&F] = &(getAnalysis<AAResultsWrapperPass>(F).getAAResults());
  }

  this->programDependenceGraph->constructNodes(M);
  constructEdgesFromUseDefs(M);
  constructEdgesFromAliases(M);

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

void llvm::PDGAnalysis::constructEdgesFromUseDefs (Module &M){
  for (auto iNodePair : programDependenceGraph->instructionNodePairs()) {
    Instruction *I = iNodePair.first;
    if (I->getNumUses() == 0)
      continue;
    for (auto& U : I->uses()) {
      auto user = U.getUser();
      if (auto userInst = dyn_cast<Instruction>(user)) {
        programDependenceGraph->addEdgeFromTo(I, userInst);
      }
    }
  }

  return ;
}

template <class iType>
void llvm::PDGAnalysis::addEdgeFromFunctionModRef (Function &F, iType *memI, CallInst *call){
  switch (aaResults[&F]->getModRefInfo(call, MemoryLocation::get(memI))) {
    case MRI_Ref:
    case MRI_Mod:
    case MRI_ModRef:
      programDependenceGraph->addEdgeFromTo(memI, call);
      break;
  }
}

template <class iType, class jType>
void llvm::PDGAnalysis::iterateInstForAliases(Function &F, jType *memJ) {
  for (auto &B : F) {
    for (auto &I : B) {
      if (auto *memI = dyn_cast<iType>(&I)) {
	auto memLocI = MemoryLocation::get(memI);
	auto memLocJ = MemoryLocation::get(memJ);
        switch (aaResults[&F]->alias(memLocI, memLocJ)) {
          case PartialAlias:
          case MayAlias:
          case MustAlias:
            programDependenceGraph->addEdgeFromTo(memI, memJ);
            break;
        }
      }
    }
  }
}

void llvm::PDGAnalysis::iterateInstForModRef(Function &F, CallInst *call) {
  for (auto &B : F) {
    for (auto &I : B) {
      if (auto *load = dyn_cast<LoadInst>(&I)) {
        addEdgeFromFunctionModRef<LoadInst>(F, load, call);
      } else if (auto *store = dyn_cast<StoreInst>(&I)) {
        addEdgeFromFunctionModRef<StoreInst>(F, store, call);
      }
    }
  }
}

void llvm::PDGAnalysis::constructEdgesFromAliases (Module &M){
  /*
   * Use alias analysis on stores, loads, and function calls to construct PDG edges
   */
  for (auto &F : M) {
    for (auto &B : F) {
      for (auto &I : B) {
        if (auto* store = dyn_cast<StoreInst>(&I)) {
          iterateInstForAliases<LoadInst, StoreInst>(F, store);
        } else if (auto *load = dyn_cast<LoadInst>(&I)) {
          iterateInstForAliases<StoreInst, LoadInst>(F, load);
        } else if (auto *call = dyn_cast<CallInst>(&I)) {
          iterateInstForModRef(F, call);
        }
      }
    }
  }
}
  
