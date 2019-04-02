/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/PostDominators.h"

#include "llvm/ADT/iterator_range.h"

#include "TalkDown.hpp"

#include "PDGAnalysis.hpp"

static cl::opt<int> Verbose("pdg-verbose", cl::ZeroOrMore, cl::Hidden, cl::desc("Verbose output (0: disabled, 1: minimal, 2: maximal"));

using namespace llvm;

bool llvm::PDGAnalysis::doInitialization (Module &M){
  this->verbose = static_cast<PDGVerbosity>(Verbose.getValue());
  return false;
}

void llvm::PDGAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<CallGraphWrapperPass>();
  AU.addRequired<AllocAA>();
  AU.addRequired<TalkDown>();
  AU.setPreservesAll();
  return ;
}

bool llvm::PDGAnalysis::runOnModule (Module &M){
  this->M = &M;
  return false;
}

llvm::PDGAnalysis::PDGAnalysis()
  : ModulePass{ID}, M{nullptr}, programDependenceGraph{nullptr}, CGUnderMain{} {}

llvm::PDGAnalysis::~PDGAnalysis() {
  if (this->programDependenceGraph)
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

  trimDGUsingCustomAliasAnalysis(pdg);

  return pdg;
}

llvm::PDG * llvm::PDGAnalysis::getPDG (){
  if (this->programDependenceGraph)
    delete this->programDependenceGraph;
  this->programDependenceGraph = new PDG();
  this->programDependenceGraph->populateNodesOf(*this->M);

  constructEdgesFromUseDefs(this->programDependenceGraph);
  constructEdgesFromAliases(this->programDependenceGraph, *this->M);
  constructEdgesFromControl(this->programDependenceGraph, *this->M);

  trimDGUsingCustomAliasAnalysis(this->programDependenceGraph);

  return this->programDependenceGraph;
}

void llvm::PDGAnalysis::trimDGUsingCustomAliasAnalysis (PDG *pdg) {

  /*
   * Invoke AllocAA
   */
  collectCGUnderFunctionMain(*this->M);
  this->allocAA = &getAnalysis<AllocAA>();
  removeEdgesNotUsedByParSchemes(pdg);

  /*
   * Invoke the TalkDown
   */
  auto& talkDown = getAnalysis<TalkDown>();
  //TODO

  return ;
}

void PDGAnalysis::collectCGUnderFunctionMain (Module &M) {
  auto main = M.getFunction("main");
  auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  std::queue<Function *> funcToTraverse;
  std::set<Function *> reached;
  funcToTraverse.push(main);
  reached.insert(main);
  while (!funcToTraverse.empty()) {
    auto func = funcToTraverse.front();
    funcToTraverse.pop();

    auto funcCGNode = callGraph[func];
    for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end())) {
      auto F = callRecord.second->getFunction();
      if (!F || F->empty()) continue;

      if (reached.find(F) != reached.end()) continue;
      reached.insert(F);
      funcToTraverse.push(F);
    }
  }

  CGUnderMain.clear();
  CGUnderMain.insert(reached.begin(), reached.end());
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

      /*
       * Check stores.
       */
      if (auto otherStore = dyn_cast<StoreInst>(&I)) {
        if (store != otherStore){
          addEdgeFromMemoryAlias<StoreInst, StoreInst>(pdg, F, AA, store, otherStore, true);
        }

      /* 
       * Check loads.
       */
      } else if (auto load = dyn_cast<LoadInst>(&I)) {
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
      if (auto store = dyn_cast<StoreInst>(&I)) {
        iterateInstForStoreAliases(pdg, F, AA, store);
      } else if (auto call = dyn_cast<CallInst>(&I)) {
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
    auto source = edge->getOutgoingT();
    if (!isa<Instruction>(source)) continue;
    auto F = cast<Instruction>(source)->getFunction();
    if (CGUnderMain.find(F) == CGUnderMain.end()) continue;
    if (edgeIsNotLoopCarriedMemoryDependency(edge)
        || edgeIsAlongNonMemoryWritingFunctions(edge)) {
      removeEdges.insert(edge);
    }
  }

  for (auto edge : removeEdges) pdg->removeEdge(edge);
}

// NOTE: Loads between random parts of separate GVs and both edges between GVs should be removed
bool llvm::PDGAnalysis::edgeIsNotLoopCarriedMemoryDependency (DGEdge<Value> *edge) {
  if (!edge->isMemoryDependence()) return false;

  auto outgoingT = edge->getOutgoingT();
  auto incomingT = edge->getIncomingT();
  if (isa<CallInst>(outgoingT) || isa<CallInst>(incomingT)) return false;

  /*
   * Assert: must be a WAR load-store OR a RAW store-load
   */
  LoadInst *load = nullptr;
  StoreInst *store = nullptr;
  if (edge->isWARDependence()) {
    assert(isa<StoreInst>(incomingT) && isa<LoadInst>(outgoingT));
    load = (LoadInst*)outgoingT;
    store = (StoreInst*)incomingT;
  } else if (edge->isRAWDependence()) {
    assert(isa<LoadInst>(incomingT) && isa<StoreInst>(outgoingT));
    store = (StoreInst*)outgoingT;
    load = (LoadInst*)incomingT;
  }

  bool loopCarried = true;
  if (isMemoryAccessIntoDifferentArrays(edge) ||
      (store && load && isBackedgeOfLoadStoreIntoSameOffsetOfArray(edge, load, store)) ||
      isBackedgeIntoSameGlobal(edge)) {
    loopCarried = false;
  }

  if (!loopCarried) {
    // NOTE: We are actually removing must dependencies, but only those that are
    // backedges where by the next iteration, the access is at a different memory location
    // assert(!edge->isMustDependence()
    //  && "LLVM AA states load store pair is a must dependence! Bad PDGAnalysis.");
    if (verbose >= PDGVerbosity::Maximal) {
      errs() << "PDGAnalysis:  Memory dependence removed! From - to:\n";
      outgoingT->print(errs() << "PDGAnalysis:  Outgoing: "); errs() << "\n";
      incomingT->print(errs() << "PDGAnalysis:  Incoming: "); errs() << "\n";
    }
  }
  return !loopCarried;
}

bool llvm::PDGAnalysis::isBackedgeOfLoadStoreIntoSameOffsetOfArray (
  DGEdge<Value> *edge,
  LoadInst *load,
  StoreInst *store
) {
  auto access1 = allocAA->getPrimitiveArrayAccess(load);
  auto access2 = allocAA->getPrimitiveArrayAccess(store);

  auto gep1 = access1.second;
  auto gep2 = access2.second;
  if (!gep1 || !gep2) return false;
  if (!allocAA->areIdenticalGEPAccessesInSameLoop(gep1, gep2)) return false;;
  if (!allocAA->areGEPIndicesConstantOrIV(gep1)) return false;

  auto outgoingI = (Instruction*)(edge->getOutgoingT());
  auto incomingI = (Instruction*)(edge->getIncomingT());
  if (canPrecedeInCurrentIteration(outgoingI, incomingI)) {
    return false;
  }

  return true;
}

bool llvm::PDGAnalysis::isBackedgeIntoSameGlobal (
  DGEdge<Value> *edge
) {
  auto access1 = allocAA->getPrimitiveArrayAccess(edge->getOutgoingT());
  auto access2 = allocAA->getPrimitiveArrayAccess(edge->getIncomingT());

  /*
   * Ensure the same global variable is accessed by the edge values
   */
  auto array1 = access1.first;
  auto array2 = access2.first;
  if (!array1 || !isa<GlobalValue>(array1)) return false;
  if (array1 != array2) return false;

  /*
   * Ensure either of the following:
   *  1) two load accesses using the same IV governed GEP
   *  2) a store into the GEP and a load of the entire GV
   */
  auto GEP1 = access1.second;
  auto GEP2 = access2.second;
  if (GEP1 && !allocAA->areGEPIndicesConstantOrIV(GEP1)) return false;
  if (GEP2 && !allocAA->areGEPIndicesConstantOrIV(GEP2)) return false;
  if (GEP1 && GEP2) {
    if (!allocAA->areIdenticalGEPAccessesInSameLoop(GEP1, GEP2)) return false;
    if (!isa<LoadInst>(edge->getOutgoingT()) ||
        !isa<LoadInst>(edge->getIncomingT())) return false;
  } else if (GEP1) {
    if (!isa<StoreInst>(edge->getOutgoingT()) ||
        !isa<LoadInst>(edge->getIncomingT())) return false;
  } else if (GEP2) {
    if (!isa<LoadInst>(edge->getOutgoingT()) ||
        !isa<StoreInst>(edge->getIncomingT())) return false;
  } else return false;

  /*
   * Ensure that the edge is a backedge
   */
  auto outgoingI = (Instruction*)(edge->getOutgoingT());
  auto incomingI = (Instruction*)(edge->getIncomingT());
  if (canPrecedeInCurrentIteration(outgoingI, incomingI)) {
    return false;
  }

  return true;
}

bool llvm::PDGAnalysis::isMemoryAccessIntoDifferentArrays (DGEdge<Value> *edge) {
  Value *array1 = allocAA->getPrimitiveArrayAccess(edge->getOutgoingT()).first;
  Value *array2 = allocAA->getPrimitiveArrayAccess(edge->getIncomingT()).first;
  return (array1 && array2 && array1 != array2);
}

bool llvm::PDGAnalysis::canPrecedeInCurrentIteration (Instruction *from, Instruction *to) {
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*from->getFunction()).getLoopInfo();
  BasicBlock *fromBB = from->getParent();
  BasicBlock *toBB = to->getParent();
  auto loop = LI.getLoopFor(fromBB);
  BasicBlock *headerBB = nullptr;
  if (loop) headerBB = loop->getHeader();

  if (fromBB == toBB) {
    for (auto &I : *fromBB) {
      if (&I == from) return true;
      if (&I == to) return false;
    }
  }

  std::queue<BasicBlock *> bbToTraverse;
  std::set<BasicBlock *> bbReached;
  auto traverseOn = [&](BasicBlock *bb) -> void {
    bbToTraverse.push(bb); bbReached.insert(bb);
  };
  traverseOn(toBB);

  while (!bbToTraverse.empty()) {
    auto bb = bbToTraverse.front();
    bbToTraverse.pop();
    if (bb == fromBB) return true;
    if (bb == headerBB) continue;

    for (auto predBB : make_range(pred_begin(bb), pred_end(bb))) {
      if (bbReached.find(predBB) == bbReached.end()) {
        traverseOn(predBB);
      }
    }
  }

  return false;
}

bool llvm::PDGAnalysis::edgeIsAlongNonMemoryWritingFunctions (DGEdge<Value> *edge) {
  if (!edge->isMemoryDependence()) return false;

  auto outgoingT = edge->getOutgoingT();
  auto incomingT = edge->getIncomingT();

  auto isFunctionMemoryless = [&](StringRef funcName) -> bool {
    return allocAA->isMemoryless(funcName);
  };
  auto isFunctionNonWriting = [&](StringRef funcName) -> bool {
    return isFunctionMemoryless(funcName) || allocAA->isReadOnly(funcName);
  };

  auto getCallFnName = [&](CallInst *call) -> StringRef {
    auto func = call->getCalledFunction();
    if (func && !func->empty()) {
      return func->getName();
    }
    return call->getCalledValue()->getName();
  };

  if (isa<CallInst>(outgoingT) && isa<CallInst>(incomingT)) {
    if (!isFunctionNonWriting(getCallFnName(cast<CallInst>(outgoingT)))) return false;
    if (!isFunctionNonWriting(getCallFnName(cast<CallInst>(incomingT)))) return false;
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

  auto callName = getCallFnName(call);
  return isa<LoadInst>(mem) && isFunctionNonWriting(callName)
    || isa<StoreInst>(mem) && isFunctionMemoryless(callName);
}
