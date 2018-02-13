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
  this->programDependenceGraph = new PDG();

  this->programDependenceGraph->constructNodes(M);
  constructEdgesFromUseDefs(M);
  constructEdgesFromAliases(M);

  return false;
}

llvm::PDGAnalysis::PDGAnalysis() : ModulePass{ID}{
  return ;
}

llvm::PDGAnalysis::~PDGAnalysis(){
  delete this->programDependenceGraph;
}

llvm::PDG * llvm::PDGAnalysis::getPDG (){
  return this->programDependenceGraph;
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
        auto *edge = programDependenceGraph->createEdgeFromTo(I, userInst);
        edge->setMemMustRaw(false, true, true);
      }
    }
  }

  return ;
}

void llvm::PDGAnalysis::addEdgeFromMemoryAlias (Function &F, AAResults *aa, Instruction *memI, Instruction *memJ, bool storePair){
  PDGEdge *edge;
  switch (aa->alias((Value *)memI,(Value *)memJ)) {
    case PartialAlias:
    case MayAlias:
      edge = programDependenceGraph->createEdgeFromTo(memI, memJ);
      edge->setMemMustRaw(true, false, !storePair);
      break;
    case MustAlias:
      edge = programDependenceGraph->createEdgeFromTo(memI, memJ);
      edge->setMemMustRaw(true, true, !storePair);
      break;
  }
}

void llvm::PDGAnalysis::addEdgeFromFunctionModRef (Function &F, AAResults *aa, StoreInst *memI, CallInst *call){
  PDGEdge *edge;
  switch (aa->getModRefInfo(call, MemoryLocation::get(memI))) {
    case MRI_Ref:
      edge = programDependenceGraph->createEdgeFromTo(memI, call);
      edge->setMemMustRaw(true, false, true);
      break;
    case MRI_Mod:
      edge = programDependenceGraph->createEdgeFromTo(memI, call);
      edge->setMemMustRaw(true, false, false);
      break;
    case MRI_ModRef:
      edge = programDependenceGraph->createEdgeFromTo(memI, call);
      edge->setMemMustRaw(true, false, true);
      edge = programDependenceGraph->createEdgeFromTo(memI, call);
      edge->setMemMustRaw(true, false, false);
      break;
  }
}

void llvm::PDGAnalysis::addEdgeFromFunctionModRef (Function &F, AAResults *aa, LoadInst *memI, CallInst *call){
  PDGEdge *edge;
  switch (aa->getModRefInfo(call, MemoryLocation::get(memI))) {
    case MRI_Ref:
      break;
    case MRI_Mod:
    case MRI_ModRef:
      edge = programDependenceGraph->createEdgeFromTo(call, memI);
      edge->setMemMustRaw(true, false, true);
      break;
  }
}

void llvm::PDGAnalysis::iterateInstForStoreAliases(Function &F, AAResults *aa, Instruction &J) {
  for (auto &B : F) {
    for (auto &I : B) {
      if (dyn_cast<StoreInst>(&I)) {
        if (&I != &J)
          addEdgeFromMemoryAlias(F, aa, &I, &J, true);
      } else if (dyn_cast<LoadInst>(&I)) {
        addEdgeFromMemoryAlias(F, aa, &J, &I, false);
      }
    }
  }
}

void llvm::PDGAnalysis::iterateInstForLoadAliases(Function &F, AAResults *aa, Instruction &J) {
  for (auto &B : F) {
    for (auto &I : B) {
      if (dyn_cast<StoreInst>(&I)) {
        addEdgeFromMemoryAlias(F, aa, &I, &J, false);
      }
    }
  }
}

void llvm::PDGAnalysis::iterateInstForModRef(Function &F, AAResults *aa, CallInst &call) {
  for (auto &B : F) {
    for (auto &I : B) {
      if (auto *load = dyn_cast<LoadInst>(&I)) {
        addEdgeFromFunctionModRef(F, aa, load, &call);
      } else if (auto *store = dyn_cast<StoreInst>(&I)) {
        addEdgeFromFunctionModRef(F, aa, store, &call);
      }
    }
  }
}

void llvm::PDGAnalysis::constructEdgesFromAliases (Module &M){
  /*
   * Use alias analysis on stores, loads, and function calls to construct PDG edges
   */
  for (auto &F : M) {
    if (F.empty()) continue ;
    auto aaResults = &(getAnalysis<AAResultsWrapperPass>(F).getAAResults());
    for (auto &B : F) {
      for (auto &I : B) {
        if (auto* store = dyn_cast<StoreInst>(&I)) {
          iterateInstForStoreAliases(F, aaResults, I);
        } else if (auto *load = dyn_cast<LoadInst>(&I)) {
          iterateInstForLoadAliases(F, aaResults, I);
        } else if (auto *call = dyn_cast<CallInst>(&I)) {
          iterateInstForModRef(F, aaResults, *call);
        }
      }
    }
  }
}
  