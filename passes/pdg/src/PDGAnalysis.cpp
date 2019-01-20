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
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/PostDominators.h"

#include "llvm/ADT/iterator_range.h"

#include "../include/PDGAnalysis.hpp"

static cl::opt<int> Verbose("pdg-verbose", cl::ZeroOrMore, cl::Hidden, cl::desc("Verbose output (0: disabled, 1: minimal, 2: maximal"));

using namespace llvm;

bool llvm::PDGAnalysis::doInitialization (Module &M){
  this->readOnlyFunctionNames = { "fprintf", "printf" };
  this->allocatorFunctionNames = { "malloc", "calloc" };
  this->memorylessFunctionNames = { "sqrt" };
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
  AU.setPreservesAll();
  return ;
}

bool llvm::PDGAnalysis::runOnModule (Module &M){
  this->programDependenceGraph = new PDG();

  this->programDependenceGraph->populateNodesOf(M);
  constructEdgesFromUseDefs(this->programDependenceGraph);
  constructEdgesFromAliases(this->programDependenceGraph, M);
  constructEdgesFromControl(this->programDependenceGraph, M);

  collectCGUnderFunctionMain(M);
  collectPrimitiveArrayValues(M);
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

void PDGAnalysis::collectFunctionCallsTo (std::set<Function *> &called, std::set<CallInst *> &calls) {
  auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  for (auto caller : CGUnderMain) {
    auto funcCGNode = callGraph[caller];
    for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end())) {
      auto F = callRecord.second->getFunction();
      if (called.find(F) == called.end()) continue;
      if (auto call = dyn_cast<CallInst>(&*callRecord.first)) {
        calls.insert(call);
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
  auto access1 = getPrimitiveArrayAccess(load, true);
  auto access2 = getPrimitiveArrayAccess(store, true);

  auto gep1 = access1.second;
  auto gep2 = access2.second;
  if (!gep1 || !gep2) return false;
  if (!areIdenticalGEPAccessesInSameLoop(gep1, gep2)) return false;

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
  auto access1 = getPrimitiveArrayAccess(edge->getOutgoingT(), true);
  auto access2 = getPrimitiveArrayAccess(edge->getIncomingT(), true);

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
  if (GEP1 && GEP2) {
    if (!areIdenticalGEPAccessesInSameLoop(GEP1, GEP2)) return false;
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
  Value *array1 = getPrimitiveArrayAccess(edge->getOutgoingT()).first;
  Value *array2 = getPrimitiveArrayAccess(edge->getIncomingT()).first;
  return (array1 && array2 && array1 != array2);
}

std::pair<Value *, GetElementPtrInst *>
llvm::PDGAnalysis::getPrimitiveArrayAccess (Value *V, bool mustBeIVGovernedAccess) {
  auto memOp = getMemoryPointerOperand(V);
  if (!memOp) return std::make_pair(nullptr, nullptr);

  /*
   * The value V is a memory instruction directly on an array
   */
  auto directAccessArray = getPrimitiveArray(memOp);
  if (directAccessArray) {
    // directAccessArray->print(errs() << "Found direct access array: "); errs() << "\n";
  }
  if (directAccessArray) return std::make_pair(directAccessArray, nullptr);

  auto empty = std::make_pair(nullptr, nullptr);
  if (auto gep = dyn_cast<GetElementPtrInst>(memOp)) {

    /*
     * Should this be requested, check that the GEP has offsets
     * determined by induction variables
     */
    if (mustBeIVGovernedAccess && !areGEPIndicesConstantOrIV(gep)) return empty;

    /*
     * The value V is a memory instruction on a GEP of either a
     * local array or a load of a global array
     */
    auto gepMemOp = gep->getPointerOperand();
    auto localArray = getLocalPrimitiveArray(gepMemOp);
    if (localArray) {
      // localArray->print(errs() << "Found GEP access local array: "); errs() << "\n";
    }
    if (localArray) return std::make_pair(localArray, gep);

    auto loadMemOp = getMemoryPointerOperand(gepMemOp);
    auto globalArray = loadMemOp
      ? getGlobalValuePrimitiveArray(loadMemOp) : nullptr;
    if (globalArray) {
      // globalArray->print(errs() << "Found GEP access global array: "); errs() << "\n";
    }
    if (globalArray) return std::make_pair(globalArray, gep);
  }
  return empty;
}

Value *llvm::PDGAnalysis::getPrimitiveArray (Value *V) {
  auto localArray = getLocalPrimitiveArray(V);
  return localArray ? localArray : getGlobalValuePrimitiveArray(V);
}

Value *llvm::PDGAnalysis::getLocalPrimitiveArray (Value *V) {
  auto targetV = V;
  if (auto cast = dyn_cast<CastInst>(V)) targetV = cast->getOperand(0);
  if (auto I = dyn_cast<Instruction>(targetV)) {
    if (primitiveArrayLocals.find(I) != primitiveArrayLocals.end()) {
      return I;
    }
  }
  return nullptr;
}

Value *llvm::PDGAnalysis::getGlobalValuePrimitiveArray (Value *V) {
  auto targetV = V;
  if (auto cast = dyn_cast<CastInst>(V)) targetV = cast->getOperand(0);
  if (auto GV = dyn_cast<GlobalValue>(targetV)) {
    if (primitiveArrayGlobals.find(GV) != primitiveArrayGlobals.end()) {
      return GV;
    }
  }
  return nullptr;
}

Value *llvm::PDGAnalysis::getMemoryPointerOperand (Value *V) {
  if (auto load = dyn_cast<LoadInst>(V)) {
    return load->getPointerOperand();
  }
  if (auto store = dyn_cast<StoreInst>(V)) {
    return store->getPointerOperand();
  }
  return nullptr;
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

/*
 * Check that all non-constant indices of GEP are those of monotonic induction variables
 */
bool llvm::PDGAnalysis::areGEPIndicesConstantOrIV (GetElementPtrInst *gep) {
  Function *gepFunc = gep->getFunction();
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*gepFunc).getLoopInfo();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*gepFunc).getSE();

  for (auto &indexV : gep->indices()) {
    if (isa<ConstantInt>(indexV)) continue;

    // Assumption? : All polynomial add recursive expressions are induction variables
    auto scev = SE.getSCEV(indexV);
    if (scev->getSCEVType() != scAddRecExpr) return false;
  }
  return true;
}

bool PDGAnalysis::areIdenticalGEPAccessesInSameLoop (GetElementPtrInst *gep1, GetElementPtrInst *gep2) {
  if (gep1 == gep2) return true;

  if (gep1->getFunction() != gep2->getFunction()) return false;
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*gep1->getFunction()).getLoopInfo();
  if (LI.getLoopFor(gep1->getParent()) != LI.getLoopFor(gep2->getParent())) return false;

  auto gepOp1 = gep1->getPointerOperand();
  auto gepOp2 = gep2->getPointerOperand();
  if (gepOp1 != gepOp2) {
    Value *accessed = nullptr;
    if (auto load = dyn_cast<LoadInst>(gepOp1)) {
      accessed = load->getPointerOperand();
    } else return false;
    if (auto load = dyn_cast<LoadInst>(gepOp2)) {
      if (accessed != load->getPointerOperand()) return false;
    } else return false;
  }

  auto indexCount = 0;
  for (auto &indexV1 : gep1->indices()) {
    auto &indexV2 = *(gep2->idx_begin() + indexCount++);
    if (indexV1 != indexV2) return false;
  }

  return true;
}

bool llvm::PDGAnalysis::collectUserInstructions (Value *V, std::set<Instruction *> &userInstructions) {
  for (auto user : V->users()) {
    Instruction *I = nullptr;
    if (isa<Instruction>(user)) {
      I = (Instruction *)user;
    } else if (isa<BitCastOperator>(user) || isa<ZExtOperator>(user)) {
      if (user->hasOneUse()) {
        auto operUser = *user->user_begin();
        if (isa<Instruction>(operUser)) {
          I = (Instruction *)operUser;
        }
      }
    }

    if (!I) return false;
    userInstructions.insert(I);
  }
  return true;
}

void llvm::PDGAnalysis::collectPrimitiveArrayValues (Module &M) {

  /*
   * Check global values used under the CG of function "main"
   */
  for (auto &GV : M.globals()) {
    if (GV.hasExternalLinkage()) continue;
    if (GV.getNumUses() == 0) continue;

    std::set<Instruction *> scopedUsers;
    if (!collectUserInstructions(&GV, scopedUsers)) continue;

    bool relevantToMain = false;
    for (auto I : scopedUsers) {
      relevantToMain |= CGUnderMain.find(I->getFunction()) != CGUnderMain.end();
      if (relevantToMain) break;
    }
    if (!relevantToMain) continue;

    if (isPrimitiveArrayPointer(&GV, scopedUsers)) primitiveArrayGlobals.insert(&GV);
  }

  /*
   * Check values where contiguous memory allocators are used
   */
  std::set<Function *> allocatorFns;
  for (auto allocName : allocatorFunctionNames) {
    auto F = M.getFunction(allocName);
    if (!F) continue;
    allocatorFns.insert(F);
  }
  std::set<CallInst *> allocatorCalls;
  collectFunctionCallsTo(allocatorFns, allocatorCalls);

  for (auto call : allocatorCalls) {
    std::set<Instruction *> allUsers;
    if (!collectUserInstructions(call, allUsers)) continue;
    if (isPrimitiveArray(call, allUsers)) primitiveArrayLocals.insert(call);
  }
}

bool PDGAnalysis::isPrimitiveArrayPointer (Value *V, std::set<Instruction *> &userInstructions) {
  bool isPrimitive = true;
  for (auto I : userInstructions) {
    if (auto store = dyn_cast<StoreInst>(I)) {
      // Confirm the store is of a contiguously allocated array unique to this value
      if (auto storedCall = dyn_cast<CallInst>(store->getValueOperand())) {
        auto callF = storedCall->getCalledFunction();
        if (allocatorFunctionNames.find(callF->getName()) != allocatorFunctionNames.end()) {
          if (storedCall->hasOneUse()) continue;
        }
      }
    }

    if (auto load = dyn_cast<LoadInst>(I)) {
      // Confirm all uses of the GV load obey those of a primitive array
      std::set<Instruction *> allUsers;
      if (collectUserInstructions(load, allUsers)
          && isPrimitiveArray(load, allUsers)) {
        continue;
      }
    }

    if (verbose >= PDGVerbosity::Maximal) {
      I->print(errs() << "PDGAnalysis:  GV related instruction not understood: "); errs() << "\n";
    }
    isPrimitive = false;
    break;
  }

  if (verbose >= PDGVerbosity::Minimal) {
    errs() << "PDGAnalysis:  GV value is a primitive integer array: " << isPrimitive << ", ";
    V->print(errs()); errs() << "\n";
  }
  return isPrimitive;
}

bool PDGAnalysis::isPrimitiveArray (Value *V, std::set<Instruction *> &userInstructions) {
  bool isPrimitive = true;
  for (auto I : userInstructions) {
    if (auto cast = dyn_cast<CastInst>(I)) {
      std::set<Instruction *> castUsers;
      if (collectUserInstructions(cast, castUsers)
          && isPrimitiveArray(cast, castUsers)) continue;
    }
    if (auto GEPUser = dyn_cast<GetElementPtrInst>(I)) {
      if (doesValueNotEscape({ GEPUser }, GEPUser)) continue;
    }
    if (auto callUser = dyn_cast<CallInst>(I)) {
      auto fnName = callUser->getCalledFunction()->getName();
      if (readOnlyFunctionNames.find(fnName) != readOnlyFunctionNames.end()) continue;
    }

    if (verbose >= PDGVerbosity::Maximal) {
      I->print(errs() << "PDGAnalysis:  related instruction not understood: "); errs() << "\n";
    }
    isPrimitive = false;
    break;
  }

  if (verbose >= PDGVerbosity::Minimal) {
    errs() << "PDGAnalysis:  value is a primitive integer array: " << isPrimitive << ", ";
    V->print(errs()); errs() << "\n";
  }
  return isPrimitive;
}

bool PDGAnalysis::doesValueNotEscape (std::set<Instruction *> checked, Instruction *I) {
  User *unkUser = nullptr;
  for (auto user : I->users()) {
    if (!isa<Instruction>(user)) {
      unkUser = user;
      break;
    }
    auto userI = cast<Instruction>(user);
    if (checked.find(userI) != checked.end()) continue;
    checked.insert(userI);

    /*
     * The termination is either local to the function, or the return value
     * is not escaped (only an integer type is returned)
     */
    if (isa<TerminatorInst>(user)) {
      if (isa<BranchInst>(user) || isa<SwitchInst>(user)) continue;
      if (isa<ReturnInst>(user)) {

        /*
         * NOTE: Technically, a program could treat the returned integer as
         * a pointer, but since at no point along the uses of the original
         * value are pointer based instructions permitted, no intentional
         * pointer value can be returned here
         */
        auto returnV = cast<ReturnInst>(user)->getReturnValue();
        if (isa<IntegerType>(returnV->getType())) continue;
      }
      unkUser = user;
      break;
    }

    /*
     * The user stores a non-escaped value into the memory location
     */
    if (auto store = dyn_cast<StoreInst>(user)) {
      auto stored = store->getValueOperand();
      auto storedDoesNotEscape = false;
      if (isa<IntegerType>(stored->getType())) {
        if (auto storedI = dyn_cast<Instruction>(stored)) {
          if (doesValueNotEscape(checked, storedI)) storedDoesNotEscape = true;
        }
        if (auto storedC = dyn_cast<ConstantData>(stored)) storedDoesNotEscape = true;
      }

      auto storage = store->getPointerOperand();
      auto storageDoesNotEscape = storage == (Value *)I;
      if (!storageDoesNotEscape) {
        if (auto storageI = dyn_cast<Instruction>(storage)) {
          if (doesValueNotEscape(checked, storageI)) storageDoesNotEscape = true;
        }
      }

      if (storedDoesNotEscape && storageDoesNotEscape) continue;
      unkUser = user;
      break;
    }

    /*
     * The user is a non-escaped integer expression
     */
    if (isa<IntegerType>(userI->getType())) {
      if (doesValueNotEscape(checked, userI)) continue;
    }
    unkUser = user;
    break;
  }

  if (unkUser) {
    if (verbose >= PDGVerbosity::Maximal) {
      unkUser->print(errs() << "PDGAnalysis:  GV related user not understood: "); errs() << "\n";
      unkUser->getType()->print(errs() << "PDGAnalysis:  \tWith type"); errs() << "\n";
    }
    return false;
  }
  return true;
}

void llvm::PDGAnalysis::collectMemorylessFunctions (Module &M) {
  for (auto F : CGUnderMain) {

    bool isMemoryless = true;
    for (auto &B : *F) {
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
    if (isMemoryless) {
      memorylessFunctionNames.insert(F->getName());
      if (verbose >= PDGVerbosity::Minimal) {
        errs() << "PDGAnalysis:  Memoryless function found: " << F->getName() << "\n";
      }
    }
  }
}

bool llvm::PDGAnalysis::edgeIsAlongNonMemoryWritingFunctions (DGEdge<Value> *edge) {
  if (!edge->isMemoryDependence()) return false;

  auto outgoingT = edge->getOutgoingT();
  auto incomingT = edge->getIncomingT();

  auto isFunctionMemoryless = [&](StringRef funcName) -> bool {
    return memorylessFunctionNames.find(funcName) != memorylessFunctionNames.end();
  };
  auto isFunctionNonWriting = [&](StringRef funcName) -> bool {
    return isFunctionMemoryless(funcName)
      || readOnlyFunctionNames.find(funcName) != readOnlyFunctionNames.end();
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
