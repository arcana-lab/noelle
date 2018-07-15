#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/PostDominators.h"

#include "llvm/ADT/iterator_range.h"

#include "../include/PDGAnalysis.hpp"

using namespace llvm;

bool llvm::PDGAnalysis::doInitialization (Module &M){
  return false;
}

void llvm::PDGAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.setPreservesAll();
  return ;
}

bool llvm::PDGAnalysis::runOnModule (Module &M){
  this->programDependenceGraph = new PDG();

  this->programDependenceGraph->addNodes(M);
  constructEdgesFromUseDefs(M);
  constructEdgesFromAliases(M);
  constructEdgesFromControl(M);

  removeApparentIntraIterationDependencies(M);

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

void llvm::PDGAnalysis::constructEdgesFromUseDefs (Module &M){
  for (auto node : make_range(programDependenceGraph->begin_nodes(), programDependenceGraph->end_nodes())) {
    Value *pdgValue = node->getT();
    if (pdgValue->getNumUses() == 0)
      continue;

    for (auto& U : pdgValue->uses()) {
      auto user = U.getUser();

      bool makeEdge = false;
      if (auto userInst = dyn_cast<Instruction>(user)) makeEdge = true;
      else if (auto userArg = dyn_cast<Argument>(user)) makeEdge = true;
      
      if (!makeEdge) continue;      
      auto *edge = programDependenceGraph->addEdge(pdgValue, user);
      edge->setMemMustType(false, true, DG_DATA_RAW);
    }
  }

  return ;
}

template <class InstI, class InstJ>
void llvm::PDGAnalysis::addEdgeFromMemoryAlias (Function &F, AAResults *aa, InstI *memI, InstJ *memJ, bool WAW){
  bool makeEdge = false, must = false;
  switch (aa->alias(MemoryLocation::get(memI), MemoryLocation::get(memJ))) {
    case PartialAlias:
    case MayAlias:
      makeEdge = true;
      break;
    case MustAlias:
      makeEdge = must = true;
      break;
  }

  if (!makeEdge) return;
  
  DataDependencyType dataDepType = WAW ? DG_DATA_WAW : DG_DATA_RAW;
  programDependenceGraph->addEdge((Value*)memI, (Value*)memJ)->setMemMustType(true, must, dataDepType);

  dataDepType = WAW ? DG_DATA_WAW : DG_DATA_WAR;
  programDependenceGraph->addEdge((Value*)memJ, (Value*)memI)->setMemMustType(true, must, dataDepType);
}

void llvm::PDGAnalysis::addEdgeFromFunctionModRef (Function &F, AAResults *aa, StoreInst *memI, CallInst *call){
  bool makeRefEdge = false, makeModEdge = false;
  switch (aa->getModRefInfo(call, MemoryLocation::get(memI))) {
    case MRI_Ref:
      makeRefEdge = true;
      break;
    case MRI_Mod:
      makeModEdge = true;
      break;
    case MRI_ModRef:
      makeRefEdge = makeModEdge = true;
      break;
  }

  if (makeRefEdge)
  {
    programDependenceGraph->addEdge((Value*)memI, (Value*)call)->setMemMustType(true, false, DG_DATA_RAW);
    programDependenceGraph->addEdge((Value*)call, (Value*)memI)->setMemMustType(true, false, DG_DATA_WAR);
  }
  if (makeModEdge)
  {
    programDependenceGraph->addEdge((Value*)memI, (Value*)call)->setMemMustType(true, false, DG_DATA_WAW);
    programDependenceGraph->addEdge((Value*)call, (Value*)memI)->setMemMustType(true, false, DG_DATA_WAW);
  }
}

void llvm::PDGAnalysis::addEdgeFromFunctionModRef (Function &F, AAResults *aa, LoadInst *memI, CallInst *call){
  bool makeRefEdge = false;
  switch (aa->getModRefInfo(call, MemoryLocation::get(memI))) {
    case MRI_Ref:
      break;
    case MRI_Mod:
    case MRI_ModRef:
      makeRefEdge = true;
      break;
  }

  if (makeRefEdge)
  {
    programDependenceGraph->addEdge((Value*)call, (Value*)memI)->setMemMustType(true, false, DG_DATA_RAW);
    programDependenceGraph->addEdge((Value*)memI, (Value*)call)->setMemMustType(true, false, DG_DATA_WAR);
  }
}

void llvm::PDGAnalysis::addEdgeFromFunctionModRef (Function &F, AAResults *aa, CallInst *otherCall, CallInst *call){
  bool makeRefEdge = false, makeModEdge = false;
  switch (aa->getModRefInfo(ImmutableCallSite(call), ImmutableCallSite(otherCall))) {
    case MRI_Ref:
      makeRefEdge = true;
      break;
    case MRI_Mod:
      makeModEdge = true;
      break;
    case MRI_ModRef:
      makeRefEdge = makeModEdge = true;
      break;
  }

  if (makeRefEdge)
  {
    programDependenceGraph->addEdge((Value*)call, (Value*)otherCall)->setMemMustType(true, false, DG_DATA_WAR);
    programDependenceGraph->addEdge((Value*)otherCall, (Value*)call)->setMemMustType(true, false, DG_DATA_RAW);
  }
  if (makeModEdge)
  {
    programDependenceGraph->addEdge((Value*)otherCall, (Value*)call)->setMemMustType(true, false, DG_DATA_WAW);
  }
}

void llvm::PDGAnalysis::iterateInstForStoreAliases(Function &F, AAResults *aa, StoreInst *store) {
  for (auto &B : F) {
    for (auto &I : B) {
      if (auto *otherStore = dyn_cast<StoreInst>(&I)) {
        if (store != otherStore)
          addEdgeFromMemoryAlias<StoreInst, StoreInst>(F, aa, store, otherStore, true);
      } else if (auto *load = dyn_cast<LoadInst>(&I)) {
        addEdgeFromMemoryAlias<StoreInst, LoadInst>(F, aa, store, load, false);
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
      } else if (auto *otherCall = dyn_cast<CallInst>(&I)) {
        addEdgeFromFunctionModRef(F, aa, otherCall, &call);
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
          iterateInstForStoreAliases(F, aaResults, store);
        } else if (auto *call = dyn_cast<CallInst>(&I)) {
          iterateInstForModRef(F, aaResults, *call);
        }
      }
    }
  }
}

void llvm::PDGAnalysis::constructEdgesFromControl (Module &M){
  for (auto &F : M) {
    if (F.empty()) continue ;
    auto postDomTree = &getAnalysis<PostDominatorTreeWrapperPass>(F).getPostDomTree();
    this->constructControlEdgesForFunction(F, *postDomTree);
  }
}

void llvm::PDGAnalysis::constructControlEdgesForFunction (Function &F, PostDominatorTree &postDomTree) {
  for (auto &B : F)
  {
    SmallVector<BasicBlock *, 10> dominatedBBs;
    postDomTree.getDescendants(&B, dominatedBBs);

    /*
     * For each basic block that B post dominates, check if B doesn't stricly post dominate its predecessor
     * If it does not, then there is a control dependency from the predecessor to B 
     */
    for (auto dominatedBB : dominatedBBs)
    {
      for (auto predBB : make_range(pred_begin(dominatedBB), pred_end(dominatedBB)))
      {
        if (postDomTree.properlyDominates(&B, predBB)) continue;
        auto controlTerminator = predBB->getTerminator();
        for (auto &I : B)
        {
          auto edge = this->programDependenceGraph->addEdge((Value*)controlTerminator, (Value*)&I);
          edge->setControl(true);
        }
      }
    }
  }
}

void llvm::PDGAnalysis::removeApparentIntraIterationDependencies (Module &M) {
  std::set<DGEdge<Value> *> removeEdges;
  for (auto edge : this->programDependenceGraph->getEdges()) {

    /*
     * Remove only WAR dependencies between intra-iteration store and load pairs
     * TODO: WAR edges only removable if load executes after store && not loop carried
     * TODO: RAW edges only removable if store executes after load && not loop carried
     */
    if (!edge->isMemoryDependence() || !edge->isWARDependence()) continue;

    auto outgoingT = edge->getOutgoingT();
    auto incomingT = edge->getIncomingT();
    
    if (!isa<StoreInst>(incomingT) || !isa<LoadInst>(outgoingT)) continue;
    LoadInst *load = (LoadInst*)outgoingT;
    StoreInst *store = (StoreInst*)incomingT;

    auto baseOp = load->getPointerOperand();
    if (load->getPointerOperand() != store->getPointerOperand()) continue;
    if (auto gep = dyn_cast<GetElementPtrInst>(baseOp)) {
      if (checkLoadStoreAliasOnSameGEP(load, store, gep)) {
        // removeEdges.insert(edge);
      }
    }
  }

  for (auto edge : removeEdges) this->programDependenceGraph->removeEdge(edge);
}

/*
 * Check that all non-constant indices of GEP are those of monotonic induction variables
 */
bool llvm::PDGAnalysis::checkLoadStoreAliasOnSameGEP (LoadInst *load, StoreInst *store, GetElementPtrInst *gep) {
  bool notAllConstantIndices = false;
  for (auto &indexV : gep->indices()) {
    if (isa<ConstantInt>(indexV)) continue;
    notAllConstantIndices = true;

    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*load->getFunction()).getSE();
    auto scev = SE.getSCEV(indexV);
    if (scev->getSCEVType() != scAddRecExpr) return false;

    auto &LI = getAnalysis<LoopInfoWrapperPass>(*load->getFunction()).getLoopInfo();
    auto loop = LI.getLoopFor(cast<Instruction>(indexV)->getParent());
    for (auto &op : loop->getHeader()->getTerminator()->operands()) {
      if (auto cmp = dyn_cast<ICmpInst>(op)) {
        auto lhs = cmp->getOperand(0);
        auto rhs = cmp->getOperand(1);
        if (!isa<ConstantInt>(lhs) && !isa<ConstantInt>(rhs)) return false;
        
        auto lhsc = SE.getSCEV(lhs);
        auto rhsc = SE.getSCEV(rhs);
        
        // auto pred = cmp->getPredicate();
        // bool isKnown = SE.isKnownViaInduction(pred, lhs, rhs);
        bool isKnown = lhsc->getSCEVType() == scAddRecExpr && rhsc->getSCEVType() == scConstant;
        isKnown |= rhsc->getSCEVType() == scAddRecExpr && lhsc->getSCEVType() == scConstant;
        if (!isKnown) return false;
        break;
      }
    }

  }
  return notAllConstantIndices;
}