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
  collectPrimitiveArrayGlobalValues(M);
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

void llvm::PDGAnalysis::removeEdgesNotUsedByParSchemes (PDG *pdg) {
  std::set<DGEdge<Value> *> removeEdges;
  for (auto edge : pdg->getEdges()) {
    auto source = edge->getOutgoingT();
    if (!isa<Instruction>(source)) continue;
    auto F = cast<Instruction>(source)->getFunction();
    if (CGUnderMain.find(F) == CGUnderMain.end()) continue;
    if (edgeIsNotLoopCarriedMemoryDependency(edge)
        || edgeIsOnKnownMemorylessFunction(edge)) {
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
  if (isMemoryAccessIntoDifferentGlobals(edge) ||
      isBackedgeOfLoadStoreIntoSameOffsetOfArray(edge, load, store) ||
      isBackedgeIntoSameGlobal(edge)) {
    loopCarried = false;
  }

  if (!loopCarried) {
    assert(!edge->isMustDependence()
      && "LLVM AA states load store pair is a must dependence! Bad PDGAnalysis.");
    // errs() << "Load store pair memory dependence removed!\n";
  }
  return !loopCarried;
}

bool llvm::PDGAnalysis::isBackedgeOfLoadStoreIntoSameOffsetOfArray (
  DGEdge<Value> *edge,
  LoadInst *load,
  StoreInst *store
) {
  // NOTE: until the GEP is guaranteed to be contiguous memory of
  //  non pointer values, no guarantee about this dependency can be made
  //  even if it is a load/store on the same IV-governed gep
  return false;

  if (!load || !store) return false;
  if (!isa<GetElementPtrInst>(load->getPointerOperand())) return false;
  if (!isa<GetElementPtrInst>(store->getPointerOperand())) return false;
  auto loadGEP = (GetElementPtrInst*)(load->getPointerOperand());
  auto storeGEP = (GetElementPtrInst*)(store->getPointerOperand());
  if (!areGEPIndicesConstantOrIV(loadGEP) ||
      !areGEPIndicesConstantOrIV(storeGEP)) return false;
  if (!areIdenticalGEPAccessesInSameLoop(loadGEP, storeGEP)) false;

  auto outgoingI = (Instruction*)(edge->getOutgoingT());
  auto incomingI = (Instruction*)(edge->getIncomingT());
  if (canPrecedeInCurrentIteration(outgoingI, incomingI)) {
    // errs() << "Not removing possible intra-iteration dependence\n";
    return false;
  }

  return true;
}

bool llvm::PDGAnalysis::isBackedgeIntoSameGlobal (
  DGEdge<Value> *edge
) {
  auto getNonAliasingGVAccess = [&](Value *V) -> std::pair<Value *, GetElementPtrInst *> {
    Value *GV = getNonAliasingGVFromDirectAccess(V);
    if (GV) return make_pair(GV, nullptr);
    return getNonAliasingGVFromGEPAccess(V, /*IVGovernedGEP*/true);
  };

  GetElementPtrInst *gep = nullptr;
  auto pair1 = getNonAliasingGVAccess(edge->getOutgoingT());
  auto pair2 = getNonAliasingGVAccess(edge->getIncomingT());

  // Ensure the same global variable is used
  auto GV1 = pair1.first;
  auto GV2 = pair2.first;
  if (!GV1 || !GV2) return false;
  if (GV1 != GV2) return false;

  // Ensure either of the following:
  //  1) two accesses using the same IV governed GEP
  //  2) a store into the GEP and a load of the entire GV
  auto GEP1 = pair1.second;
  auto GEP2 = pair2.second;
  if (GEP1 && GEP2) {
    if (!areIdenticalGEPAccessesInSameLoop(GEP1, GEP2)) return false;
  } else if (GEP1) {
    if (!isa<StoreInst>(edge->getOutgoingT()) ||
        !isa<LoadInst>(edge->getIncomingT())) return false;
  } else if (GEP2) {
    if (!isa<LoadInst>(edge->getOutgoingT()) ||
        !isa<StoreInst>(edge->getIncomingT())) return false;
  } else return false;

  auto outgoingI = (Instruction*)(edge->getOutgoingT());
  auto incomingI = (Instruction*)(edge->getIncomingT());
  if (canPrecedeInCurrentIteration(outgoingI, incomingI)) {
    // errs() << "Not removing possible intra-iteration dependence\n";
    return false;
  }

  return true;
}

bool llvm::PDGAnalysis::isMemoryAccessIntoDifferentGlobals (DGEdge<Value> *edge) {
  auto getNonAliasingGVAccess = [&](Value *V) -> Value * {
    Value *GV = getNonAliasingGVFromDirectAccess(V);
    if (GV) return GV;
    return getNonAliasingGVFromGEPAccess(V, /*IVGovernedGEP*/false).first;
  };

  Value *GV1 = getNonAliasingGVAccess(edge->getOutgoingT());
  Value *GV2 = getNonAliasingGVAccess(edge->getIncomingT());
  if (!GV1 || !GV2) return false;
  if (GV1 == GV2) return false;
  return true;
}

Value *llvm::PDGAnalysis::getNonAliasingGVFromDirectAccess (Value *V) {
  auto memOp = getMemoryPointerOp(V);
  return memOp ? getGVIfNonAliasing(memOp) : nullptr;
}

std::pair<Value *, GetElementPtrInst *>
llvm::PDGAnalysis::getNonAliasingGVFromGEPAccess (Value *V, bool IVGovernedGEP) {
  auto empty = std::make_pair(nullptr, nullptr);
  auto memOp = getMemoryPointerOp(V);
  if (!memOp) return empty;
  if (auto gep = dyn_cast<GetElementPtrInst>(memOp)) {
    if (IVGovernedGEP && !areGEPIndicesConstantOrIV(gep)) return empty;
    auto memOp = getMemoryPointerOp(gep->getPointerOperand());
    if (memOp) return std::make_pair(getGVIfNonAliasing(memOp), gep);
  }
  return empty;
}

Value *llvm::PDGAnalysis::getGVIfNonAliasing (Value *V) {
  if (auto GV = dyn_cast<GlobalValue>(V)) {
    if (primitiveArrayGlobals.find(GV) != primitiveArrayGlobals.end()) {
      return GV;
    }
  }
  return nullptr;
}

Value *llvm::PDGAnalysis::getMemoryPointerOp (Value *V) {
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

  Value *accessed = nullptr;
  if (auto load = dyn_cast<LoadInst>(gep1->getPointerOperand())) {
    accessed = load->getPointerOperand();
  } else return false;
  if (auto load = dyn_cast<LoadInst>(gep2->getPointerOperand())) {
    if (accessed != load->getPointerOperand()) return false;
  } else return false;

  auto indexCount = 0;
  for (auto &indexV1 : gep1->indices()) {
    auto &indexV2 = *(gep2->idx_begin() + indexCount++);
    if (indexV1 != indexV2) return false;
  }

  return true;
}

void llvm::PDGAnalysis::collectPrimitiveArrayGlobalValues (Module &M) {
  std::set<std::string> allocators = { "malloc", "calloc" };
  std::set<std::string> readOnlyFns = { "fprintf", "printf" };
  for (auto &GV : M.globals()) {
    if (GV.hasExternalLinkage()) continue;
    bool isPrimitiveArray = true;
    bool usedByMain = false;

    for (auto user : GV.users()) {
      if (auto I = dyn_cast<Instruction>(user)) {
        if (CGUnderMain.find(I->getFunction()) != CGUnderMain.end()) {
          usedByMain = true;
        }

        if (auto store = dyn_cast<StoreInst>(I)) {
          // Confirm the store is of a malloc'd or calloc'd array, one that is
          //  only stored into this value
          if (auto storedCall = dyn_cast<CallInst>(store->getValueOperand())) {
            auto callF = storedCall->getCalledFunction();
            if (allocators.find(callF->getName()) != allocators.end()) {
              if (storedCall->hasOneUse()) continue;
            }
          }
          // store->print(errs() << "PDGAnalysis:  GV store: "); errs() << "\n";
        }
        if (auto load = dyn_cast<LoadInst>(I)) {
          // Confirm all uses of the GV load are GEP that are used to store
          //  non-addressed values only, or read only function calls
          bool nonAddressedUsers = true;
          for (auto loadUser : load->users()) {
            if (auto GEPUser = dyn_cast<GetElementPtrInst>(loadUser)) {
              if (isOnlyUsedByNonAddrValues({}, GEPUser)) continue;
            }
            if (auto callUser = dyn_cast<CallInst>(loadUser)) {
              auto fnName = callUser->getCalledFunction()->getName();
              if (readOnlyFns.find(fnName) != readOnlyFns.end()) continue;
              // errs() << "PDGAnalysis:  Unnaccepted name: " << fnName << "\n";
            }
            // loadUser->print(errs() << "PDGAnalysis:  GV load user: "); errs() << "\n";
            nonAddressedUsers = false;
          }
          if (nonAddressedUsers) continue;
        }
        // I->print(errs() << "PDGAnalysis:  GV unknown userI: "); errs() << "\n";
      }

      if (auto oper = dyn_cast<Operator>(user)) {
        if (isa<BitCastOperator>(user) || isa<ZExtOperator>(user)) continue;
        if (isa<GEPOperator>(user) || isa<PtrToIntOperator>(user)
            || isa<OverflowingBinaryOperator>(user) || isa<PossiblyExactOperator>(user)) {
          isPrimitiveArray = false;
        }
        // user->print(errs() << "PDGAnalysis:  operator user not understood: "); errs() << "\n";
      }

      // user->print(errs() << "PDGAnalysis:  user not understood: "); errs() << "\n";
      isPrimitiveArray = false;
    }

    if (!usedByMain) continue;
    if (!isPrimitiveArray) {
      // GV.print(errs() << "GV not understood to be primitive integer array: "); errs() << "\n";
      continue;
    }
    // GV.print(errs() << "GV understood to be primitive integer array: "); errs() << "\n";
    primitiveArrayGlobals.insert(&GV);
  }
}

bool PDGAnalysis::isOnlyUsedByNonAddrValues (std::set<Instruction *> checked, Instruction *I) {
  if (checked.find(I) != checked.end()) return true;
  checked.insert(I);

  for (auto user : I->users()) {
    if (isa<TerminatorInst>(user)) continue;
    if (auto store = dyn_cast<StoreInst>(user)) {
      auto stored = store->getValueOperand();
      if (isa<IntegerType>(stored->getType())) {
        if (auto storedI = dyn_cast<Instruction>(stored)) {
          if (isOnlyUsedByNonAddrValues(checked, storedI)) continue;
        }
        if (auto storedC = dyn_cast<ConstantData>(stored)) continue;
      }
      // stored->print(errs() << "PDGAnalysis: store not integer type or used by such: "); errs() << "\n";
      // stored->getType()->print(errs() << "TYPE: "); errs() << "\n";
    }
    if (auto userI = dyn_cast<Instruction>(user)) {
      if (isa<IntegerType>(userI->getType())) {
        if (isOnlyUsedByNonAddrValues(checked, userI)) continue;
      }
      // userI->print(errs() << "PDGAnalysis: user not integer type or used by such: "); errs() << "\n";
      // userI->getType()->print(errs() << "TYPE: "); errs() << "\n";
    }
    // user->print(errs() << "PDGAnalysis: Inst user not understood: "); errs() << "\n";
    return false;
  }
  return true;
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

