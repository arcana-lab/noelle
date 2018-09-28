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
  this->memorylessFunctionNames = {
    "sqrt"
  };
  return false;
}

void llvm::PDGAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.setPreservesAll();
  return ;
}

bool llvm::PDGAnalysis::runOnModule (Module &M){
  this->programDependenceGraph = new PDG();

  this->programDependenceGraph->populateNodesOf(M);
  constructEdgesFromUseDefs(this->programDependenceGraph);
  constructEdgesFromAliases(this->programDependenceGraph, M);
  constructEdgesFromControl(this->programDependenceGraph, M);

  collectMemorylessFunctions(M);
  removeEdgesNotUsedByParSchemes(this->programDependenceGraph);

  return false;
}

llvm::PDGAnalysis::PDGAnalysis() : ModulePass{ID}{
  return ;
}

llvm::PDGAnalysis::~PDGAnalysis(){
  delete this->programDependenceGraph;
}

llvm::PDG * llvm::PDGAnalysis::getFunctionPDG (Function &F) {
  auto pdg = new PDG();
  pdg->populateNodesOf(F);

  auto &AA = getAnalysis<AAResultsWrapperPass>(F).getAAResults();
  constructEdgesFromUseDefs(pdg);
  constructEdgesFromAliasesForFunction(pdg, F, AA);
  auto &PDT = getAnalysis<PostDominatorTreeWrapperPass>(F).getPostDomTree();
  constructEdgesFromControlForFunction(pdg, F, PDT);

  return pdg;
}

llvm::PDG * llvm::PDGAnalysis::getPDG (){
  return this->programDependenceGraph;
}

void llvm::PDGAnalysis::constructEdgesFromUseDefs (PDG *pdg){
  for (auto node : make_range(pdg->begin_nodes(), pdg->end_nodes())) {
    Value *pdgValue = node->getT();
    if (pdgValue->getNumUses() == 0)
      continue;

    for (auto& U : pdgValue->uses()) {
      auto user = U.getUser();

      if (isa<Instruction>(user) || isa<Argument>(user)) {
        auto edge = pdg->addEdge(pdgValue, user);
        edge->setMemMustType(false, true, DG_DATA_RAW);
      }
    }
  }
}

template <class InstI, class InstJ>
void llvm::PDGAnalysis::addEdgeFromMemoryAlias (PDG *pdg, Function &F, AAResults &AA, InstI *memI, InstJ *memJ, bool WAW){
  bool makeEdge = false, must = false;
  switch (AA.alias(MemoryLocation::get(memI), MemoryLocation::get(memJ))) {
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
  pdg->addEdge((Value*)memI, (Value*)memJ)->setMemMustType(true, must, dataDepType);

  dataDepType = WAW ? DG_DATA_WAW : DG_DATA_WAR;
  pdg->addEdge((Value*)memJ, (Value*)memI)->setMemMustType(true, must, dataDepType);
}

void llvm::PDGAnalysis::addEdgeFromFunctionModRef (PDG *pdg, Function &F, AAResults &AA, StoreInst *memI, CallInst *call){
  bool makeRefEdge = false, makeModEdge = false;
  switch (AA.getModRefInfo(call, MemoryLocation::get(memI))) {
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
    pdg->addEdge((Value*)memI, (Value*)call)->setMemMustType(true, false, DG_DATA_RAW);
    pdg->addEdge((Value*)call, (Value*)memI)->setMemMustType(true, false, DG_DATA_WAR);
  }
  if (makeModEdge)
  {
    pdg->addEdge((Value*)memI, (Value*)call)->setMemMustType(true, false, DG_DATA_WAW);
    pdg->addEdge((Value*)call, (Value*)memI)->setMemMustType(true, false, DG_DATA_WAW);
  }
}

void llvm::PDGAnalysis::addEdgeFromFunctionModRef (PDG *pdg, Function &F, AAResults &AA, LoadInst *memI, CallInst *call){
  bool makeModEdge = false;
  switch (AA.getModRefInfo(call, MemoryLocation::get(memI))) {
    case MRI_Ref:
      break;
    case MRI_Mod:
    case MRI_ModRef:
      makeModEdge = true;
      break;
  }

  if (makeModEdge)
  {
    pdg->addEdge((Value*)call, (Value*)memI)->setMemMustType(true, false, DG_DATA_RAW);
    pdg->addEdge((Value*)memI, (Value*)call)->setMemMustType(true, false, DG_DATA_WAR);
  }
}

void llvm::PDGAnalysis::addEdgeFromFunctionModRef (PDG *pdg, Function &F, AAResults &AA, CallInst *otherCall, CallInst *call){
  bool makeRefEdge = false, makeModEdge = false;
  switch (AA.getModRefInfo(ImmutableCallSite(call), ImmutableCallSite(otherCall))) {
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
    pdg->addEdge((Value*)call, (Value*)otherCall)->setMemMustType(true, false, DG_DATA_WAR);
    pdg->addEdge((Value*)otherCall, (Value*)call)->setMemMustType(true, false, DG_DATA_RAW);
  }
  if (makeModEdge)
  {
    pdg->addEdge((Value*)otherCall, (Value*)call)->setMemMustType(true, false, DG_DATA_WAW);
  }
}

void llvm::PDGAnalysis::iterateInstForStoreAliases (PDG *pdg, Function &F, AAResults &AA, StoreInst *store) {
  for (auto &B : F) {
    for (auto &I : B) {
      if (auto *otherStore = dyn_cast<StoreInst>(&I)) {
        if (store != otherStore)
          addEdgeFromMemoryAlias<StoreInst, StoreInst>(pdg, F, AA, store, otherStore, true);
      } else if (auto *load = dyn_cast<LoadInst>(&I)) {
        addEdgeFromMemoryAlias<StoreInst, LoadInst>(pdg, F, AA, store, load, false);
      }
    }
  }
}

void llvm::PDGAnalysis::iterateInstForModRef(PDG *pdg, Function &F, AAResults &AA, CallInst &call) {
  for (auto &B : F) {
    for (auto &I : B) {
      if (auto *load = dyn_cast<LoadInst>(&I)) {
        addEdgeFromFunctionModRef(pdg, F, AA, load, &call);
      } else if (auto *store = dyn_cast<StoreInst>(&I)) {
        addEdgeFromFunctionModRef(pdg, F, AA, store, &call);
      } else if (auto *otherCall = dyn_cast<CallInst>(&I)) {
        addEdgeFromFunctionModRef(pdg, F, AA, otherCall, &call);
      }
    }
  }
}

void llvm::PDGAnalysis::constructEdgesFromAliases (PDG *pdg, Module &M){
  /*
   * Use alias analysis on stores, loads, and function calls to construct PDG edges
   */
  for (auto &F : M) {
    if (F.empty()) continue ;
    auto &AA = getAnalysis<AAResultsWrapperPass>(F).getAAResults();
    constructEdgesFromAliasesForFunction(pdg, F, AA);
  }
}

void llvm::PDGAnalysis::constructEdgesFromAliasesForFunction (PDG *pdg, Function &F, AAResults &AA){
  for (auto &B : F) {
    for (auto &I : B) {
      if (auto* store = dyn_cast<StoreInst>(&I)) {
        iterateInstForStoreAliases(pdg, F, AA, store);
      } else if (auto *call = dyn_cast<CallInst>(&I)) {
        iterateInstForModRef(pdg, F, AA, *call);
      }
    }
  }
}

void llvm::PDGAnalysis::constructEdgesFromControl (PDG *pdg, Module &M){
  for (auto &F : M) {
    if (F.empty()) continue ;
    auto &postDomTree = getAnalysis<PostDominatorTreeWrapperPass>(F).getPostDomTree();
    this->constructEdgesFromControlForFunction(pdg, F, postDomTree);
  }
}

void llvm::PDGAnalysis::constructEdgesFromControlForFunction (PDG *pdg, Function &F, PostDominatorTree &postDomTree) {
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
          auto edge = pdg->addEdge((Value*)controlTerminator, (Value*)&I);
          edge->setControl(true);
        }
      }
    }
  }
}

void llvm::PDGAnalysis::removeEdgesNotUsedByParSchemes (PDG *pdg) {
  std::set<DGEdge<Value> *> removeEdges;
  for (auto edge : pdg->getEdges()) {
    if (edgeIsApparentIntraIterationDependency(edge)
        || edgeIsOnKnownMemorylessFunction(edge)) {
      removeEdges.insert(edge);
    }
  }

  for (auto edge : removeEdges) pdg->removeEdge(edge);
}

/*
 * Remove dependencies between intra-iteration store and load pairs
 */
bool llvm::PDGAnalysis::edgeIsApparentIntraIterationDependency (DGEdge<Value> *edge) {
  if (!edge->isMemoryDependence() || edge->isWAWDependence()) return false;

  auto outgoingT = edge->getOutgoingT();
  auto incomingT = edge->getIncomingT();
  if (isa<CallInst>(outgoingT) || isa<CallInst>(incomingT)) return false;

  /*
   * Assert: must be a WAR load-store OR a RAW store-load
   */
  LoadInst *load;
  StoreInst *store;
  if (edge->isWARDependence()) {
    assert(isa<StoreInst>(incomingT) && isa<LoadInst>(outgoingT));
    load = (LoadInst*)outgoingT;
    store = (StoreInst*)incomingT;
  } else {
    assert(isa<LoadInst>(incomingT) && isa<StoreInst>(outgoingT));
    store = (StoreInst*)outgoingT;
    load = (LoadInst*)incomingT;
  }

  auto baseOp = load->getPointerOperand();
  if (load->getPointerOperand() != store->getPointerOperand()) return false;
  if (auto gep = dyn_cast<GetElementPtrInst>(baseOp)) {

    /*
     * Skip if the edge may be intra-iteration
     */
    if (instMayPrecede(outgoingT, incomingT)) return false;

    if (checkLoadStoreAliasOnSameGEP(gep)) {
      return true;
    }
  }

  return false;
}

bool llvm::PDGAnalysis::instMayPrecede (Value *from, Value *to) {
  auto fromI = (Instruction*)from;
  auto toI = (Instruction*)to;
  auto& DT = getAnalysis<DominatorTreeWrapperPass>(*fromI->getFunction()).getDomTree();
  BasicBlock *fromBB = fromI->getParent();

  if (fromBB == toI->getParent()) {
    for (auto &I : *fromBB) {
      if (&I == fromI) return true;
      if (&I == toI) return false;
    }
  }

  std::queue<DGNode<Value> *> controlNodes;
  std::set<DGNode<Value> *> visitedNodes;
  auto startNode = this->programDependenceGraph->fetchNode(to);
  controlNodes.push(startNode);
  visitedNodes.insert(startNode);
  while (!controlNodes.empty()) {
    auto node = controlNodes.front();
    controlNodes.pop();

    BasicBlock *bb = ((Instruction*)node->getT())->getParent();
    if (DT.dominates(fromBB, bb)) return true;
    for (auto edge : node->getIncomingEdges()) {
      if (!edge->isControlDependence()) continue;
      auto incomingNode = edge->getOutgoingNode();
      if (visitedNodes.find(incomingNode) != visitedNodes.end()) continue;
      controlNodes.push(incomingNode);
      visitedNodes.insert(incomingNode);
    }
  }
  return false;
}

/*
 * Check that all non-constant indices of GEP are those of monotonic induction variables
 */
bool llvm::PDGAnalysis::checkLoadStoreAliasOnSameGEP (GetElementPtrInst *gep) {
  Function *gepFunc = gep->getFunction();
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*gepFunc).getLoopInfo();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*gepFunc).getSE();

  auto notAllConstantIndices = false;
  for (auto &indexV : gep->indices()) {
    if (isa<ConstantInt>(indexV)) continue;
    notAllConstantIndices = true;

    auto scev = SE.getSCEV(indexV);
    if (scev->getSCEVType() != scAddRecExpr) return false;

    // Assumption? : All polynomial add recursive expressions are induction variables
    // auto loop = LI.getLoopFor(cast<Instruction>(indexV)->getParent());
    // for (auto &op : loop->getHeader()->getTerminator()->operands()) {
    //   if (auto cmp = dyn_cast<ICmpInst>(op)) {
    //     auto lhs = cmp->getOperand(0);
    //     auto rhs = cmp->getOperand(1);
    //     if (!isa<ConstantInt>(lhs) && !isa<ConstantInt>(rhs)) return false;
    //     auto lhsc = SE.getSCEV(lhs);
    //     auto rhsc = SE.getSCEV(rhs);
    //     
    //     // auto pred = cmp->getPredicate();
    //     // bool isKnown = SE.isKnownViaInduction(pred, lhs, rhs);
    //     bool isKnown = lhsc->getSCEVType() == scAddRecExpr && rhsc->getSCEVType() == scConstant;
    //     isKnown |= rhsc->getSCEVType() == scAddRecExpr && lhsc->getSCEVType() == scConstant;
    //     if (!isKnown) return false;
    //     break;
    //   }
    // }
  }

  return notAllConstantIndices;
}

void llvm::PDGAnalysis::collectMemorylessFunctions (Module &M) {
  for (auto &F : M) {
    if (F.empty()) continue;

    bool isMemoryless = true;
    for (auto &B : F) {
      for (auto &I : B) {
        if (isa<LoadInst>(I) || isa<StoreInst>(I) || isa<CallInst>(I)) {
          isMemoryless = false;
        }

        for (auto &op : I.operands()) {
          if (isa<GlobalValue>(op.get())) {
            isMemoryless = false;
            break;
          }
        }
        
        if (!isMemoryless) break;
      }

      if (!isMemoryless) break;
    }

    // TODO(angelo): Trigger a recheck of functions using this function
    // in case they are then found to be memoryless
    if (isMemoryless) memorylessFunctionNames.insert(F.getName());
  }
}

bool llvm::PDGAnalysis::edgeIsOnKnownMemorylessFunction (DGEdge<Value> *edge) {
  if (!edge->isMemoryDependence()) return false;

  auto outgoingT = edge->getOutgoingT();
  auto incomingT = edge->getIncomingT();

  auto isCallMemoryless = [&](CallInst *call) -> bool {
    auto func = call->getCalledFunction();
    if (func && !func->empty()) {
      auto funcName = func->getName();
      return memorylessFunctionNames.find(funcName) != memorylessFunctionNames.end();
    }

    auto funcVal = call->getCalledValue();
    auto funcName = funcVal->getName();
    if (memorylessFunctionNames.find(funcName) != memorylessFunctionNames.end()) {
      return true;
    }
    return false;
  };

  if (isa<CallInst>(outgoingT) && isa<CallInst>(incomingT)) {
    if (!isCallMemoryless(cast<CallInst>(outgoingT))) return false;
    if (!isCallMemoryless(cast<CallInst>(incomingT))) return false;
    return true;
  }

  CallInst *call;
  Value *mem;
  if (isa<CallInst>(outgoingT)) {
    call = cast<CallInst>(outgoingT);
    mem = incomingT;
  } else if (isa<CallInst>(incomingT)) {
    call = cast<CallInst>(incomingT);
    mem = outgoingT; 
  } else {
    return false;
  }

  for (auto i = 0; i < call->getNumOperands(); ++i) {
    if (mem == call->getOperand(i)) return false;
  }
  return isCallMemoryless(call);
}

