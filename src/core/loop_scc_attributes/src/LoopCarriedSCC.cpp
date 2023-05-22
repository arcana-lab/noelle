/*
 * Copyright 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/LoopCarriedSCC.hpp"

namespace llvm::noelle {

LoopCarriedSCC::LoopCarriedSCC(
    SCCKind K,
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value> *> &loopCarriedDependences,
    bool commutative)
  : GenericSCC(K, s, loop),
    lcDeps{ loopCarriedDependences },
    _commutative{ commutative || this->isLoadStoreCommutative(s) } {
  return;
}

std::set<DGEdge<Value> *> LoopCarriedSCC::getLoopCarriedDependences(
    void) const {
  return this->lcDeps;
}

bool LoopCarriedSCC::isCommutative(void) const {
  return this->_commutative;
}

bool LoopCarriedSCC::isLoadStoreCommutative(SCC *scc) {
  /**
   *  Detect the following pattern
   *  x = load %p
   *  <x> = add <type> <x>, <y>
   *  store %p x
   * */
  errs() << "NIKHIL CHECKING FOR LOAD STORE COMMUTATIVITY\n";
  std::vector<LoadInst *> loadInsts;

  for (auto it = scc->begin_internal_node_map();
       it != scc->end_internal_node_map();
       it++) {
    // auto nodeptr = *it;
    llvm::Value *inst = it->first;

    if (isa<LoadInst>(inst)) {
      loadInsts.push_back(cast<LoadInst>(inst));

      // valToDGNode[inst] = nodeptr;
    }
  }

  if (loadInsts.empty()) {
    /**
     * For now, we don't handle the case when there's no load inst in the SCC
     *  though in the future this can be considered
     */
    return false;
  }

  std::map<LoadInst *, Instruction::BinaryOps> loadToFollowingOp;
  std::map<LoadInst *, std::set<llvm::StoreInst *>> loadToFollowingStore;

  for (LoadInst *load : loadInsts) {
    Value *curVal = cast<Value>(load);

    bool traceResult = this->LoadToStoreTrace(scc,
                                              curVal,
                                              load,
                                              loadToFollowingOp,
                                              loadToFollowingStore);

    if (!traceResult) {
      return false;
    }
  }

  errs() << "SCC Commutative: "
         << " done load store trace\n";
  /**
   *  For every two LoadStore Combo, if they depend on each other, they must
   * have compatible bin operations to be commutative.
   * */

  for (uint32_t i = 0; i < loadInsts.size(); i++) {
    for (uint32_t j = i + 1; j < loadInsts.size(); j++) {

      /* the ops follow the loads must form a compatible set */
      LoadInst *curload = loadInsts[i];
      LoadInst *preload = loadInsts[j];

      bool loadNoDependence = this->loadStoreNoDependence(scc,
                                                          curload,
                                                          preload,
                                                          loadToFollowingStore);

      errs()
          << "SCC Commutative: loadNoDependence = " << loadNoDependence << '\n';
      if (loadNoDependence) {
        continue;
      }

      errs() << "NIKHIL COUNTING IN MAP\n";
      bool curloadHasOp = loadToFollowingOp.count(curload);
      bool preloadHasOp = loadToFollowingOp.count(preload);
      errs() << "NIKHIL COUNTED IN MAP\n";

      if (curloadHasOp != preloadHasOp) {
        /* two load must at the same time either have op followed or not */
        return false;
      }

      if (!curloadHasOp && !preloadHasOp) {
        /* both no op folllowed. This is fine */
        continue;
      }

      /* here curloadHasOp == preloadHasOp == true */

      errs() << "CHKING IF BIN OPS COMPATIBLE\n";
      bool compatible =
          this->binOpsCommCompatible(loadToFollowingOp.at(curload),
                                     loadToFollowingOp.at(preload));

      errs() << "CHECKED BINOPS COMPATIBLE\n";

      /* not from the compatible set */
      if (!compatible) {
        return false;
      }
    }
  }
  errs() << "RETURNING FROM FUNCTION\n";
  return true;
}

bool LoopCarriedSCC::loadStoreNoDependence(
    SCC *scc,
    LoadInst *load1,
    LoadInst *load2,
    std::map<llvm::LoadInst *, std::set<llvm::StoreInst *>>
        &loadToFollowingStore
    // std::map<llvm::Value *, DGNode<llvm::Value> *> & valToDGNode
) {
  /**
   *  load1
   *  store1
   *
   *  load2
   *  store2
   *
   *  check if load2 depends on store1 or load2 depends on store2
   * */

  Value *ptr1 = load1->getPointerOperand();
  Value *ptr2 = load2->getPointerOperand();

  auto load1Node = scc->fetchNode(load1);
  auto load2Node = scc->fetchNode(load2);

  for (StoreInst *store : loadToFollowingStore[load1]) {
    auto storeNode = scc->fetchNode(store);

    auto connectedEdges = scc->fetchEdges(storeNode, load2Node);

    /**
     *  existed connected edges
     * */
    if (!connectedEdges.empty()) {
      return false;
    }
  }

  /**
   *  same logic applies again to the other pair
   * */

  for (StoreInst *store : loadToFollowingStore[load2]) {
    auto storeNode = scc->fetchNode(store);

    auto connectedEdges = scc->fetchEdges(storeNode, load1Node);

    /**
     *  existed connected edges
     * */
    if (!connectedEdges.empty()) {
      return false;
    }
  }

  return true;
}

bool LoopCarriedSCC::LoadToStoreTrace(
    SCC *scc,       /* input scc */
    Value *curval,  /* changed for every recursion */
    LoadInst *load, /* input */
    std::map<LoadInst *, Instruction::BinaryOps>
        &loadToFollowingOp, /* output contains load to bin ops followed. used to
                               tell if SCC is commutative when there's multiple
                               loads*/
    std::map<LoadInst *, std::set<llvm::StoreInst *>>
        &loadToFollowingStore /* output contains load to store followed */
) {
  // errs() << "SCC Commutative: " << " visit " << *curval << '\n';

  if (!scc->isInternal(curval)) {
    return false;
  }

  auto curNode = scc->fetchNode(curval);
  std::set<Value *> nextInternalNodes;

  /**
   *  find all internal nodes
   * */

  // errs() << "SCC Commutative: from " << *curval << " at " << curval<< " \n";
  for (auto it : curNode->getOutgoingEdges()) {
    Value *nextVal = it->getIncomingT();

    if (scc->isInternal(nextVal)) {
      nextInternalNodes.insert(nextVal);
    }
  }

  if (nextInternalNodes.empty()) {
    return false;
  }

  for (Value *nextVal : nextInternalNodes) {
    if (!isa<StoreInst>(nextVal) && !isa<BinaryOperator>(nextVal)) {
      errs()
          << "SCC Commutative: out going edges to instructions other than store and binOp!\n";
      errs()
          << "SCC Commutative: from " << *curval << " at " << curval << " \n";
      errs()
          << "SCC Commutative: to   " << *nextVal << " at " << nextVal << " \n";
      return false;
    }

    if (isa<StoreInst>(nextVal)) {
      StoreInst *store = cast<StoreInst>(nextVal);

      bool loadDependOnStore = this->isSCCInternalEdge(scc, store, load);
      if (!loadDependOnStore) {
        /**
         *  if store the value to a pointer other than the load pointer
         *    this is likely to escape and we don't consider for now
         * */
        return false;
      }

      if (store->getPointerOperand() == load->getPointerOperand()) {
        /* record the store with the load */
        loadToFollowingStore[load].insert(store);
      }
    } else if (isa<BinaryOperator>(nextVal)) {
      BinaryOperator *binOp = cast<BinaryOperator>(nextVal);
      if (LoadInst *prevLoad = dyn_cast<LoadInst>(curval)) {
        /**
         * If this is a load, then we recurse to next instruction
         */
        bool traceResult = this->LoadToStoreTrace(scc,
                                                  binOp,
                                                  load,
                                                  loadToFollowingOp,
                                                  loadToFollowingStore);

        if (!traceResult) {
          return false;
        }

        loadToFollowingOp[prevLoad] = binOp->getOpcode();

      } else if (BinaryOperator *prevBinOp = dyn_cast<BinaryOperator>(curval)) {
        /**
         * Only continue to recurse if the two binary operations are compatible
         *  TODO: think more about this
         */

        // Instruction::BinaryOps::Add
        bool compatible = this->binOpsCommCompatible(binOp->getOpcode(),
                                                     prevBinOp->getOpcode());

        if (!compatible) {
          return false;
        }

        bool traceResult = this->LoadToStoreTrace(scc,
                                                  binOp,
                                                  load,
                                                  loadToFollowingOp,
                                                  loadToFollowingStore);

        if (!traceResult) {
          return false;
        }
      } else {
        /* we don't handle cases other than bin op and load */
        return false;
      }

    } else {
      /* shouldn't enter here */
      return false;
    }
  }

  if (loadToFollowingStore[load].empty()) {
    /* no corresponding store to the load */
    return false;
  }
  return true;
}

bool LoopCarriedSCC::binOpsCommCompatible(Instruction::BinaryOps op1,
                                          Instruction::BinaryOps op2) {
  if (this->compatibleBinOps.count(op1) && this->compatibleBinOps.count(op2)) {
    if (this->compatibleBinOps.at(op1).count(op2)
        && this->compatibleBinOps.at(op2).count(op1)) {
      return true;
    }
  }
  return false;
}

bool LoopCarriedSCC::isSCCInternalEdge(SCC *scc, Value *from, Value *to) {
  if (!scc->isInternal(from)) {
    return false;
  }
  if (!scc->isInternal(to)) {
    return false;
  }

  auto fromNode = scc->fetchNode(from);
  auto toNode = scc->fetchNode(to);

  auto edgeSet = scc->fetchEdges(fromNode, toNode);

  return !edgeSet.empty();
}

const std::map<Instruction::BinaryOps, std::set<Instruction::BinaryOps>>
    LoopCarriedSCC::compatibleBinOps = {
      { Instruction::BinaryOps::Add,
        { Instruction::BinaryOps::Add, Instruction::BinaryOps::Sub } },

      { Instruction::BinaryOps::Sub,
        { Instruction::BinaryOps::Add, Instruction::BinaryOps::Sub } },

      { Instruction::BinaryOps::Mul, { Instruction::BinaryOps::Mul } },

      { Instruction::BinaryOps::UDiv, { Instruction::BinaryOps::UDiv } },

      { Instruction::BinaryOps::SDiv, { Instruction::BinaryOps::SDiv } },

      /**
       *  Floating point operations
       * */
      { Instruction::BinaryOps::FAdd,
        { Instruction::BinaryOps::FAdd, Instruction::BinaryOps::FSub } },

      { Instruction::BinaryOps::FSub,
        { Instruction::BinaryOps::FAdd, Instruction::BinaryOps::FSub } },

      { Instruction::BinaryOps::FMul,
        { Instruction::BinaryOps::FMul, Instruction::BinaryOps::FDiv } },

      { Instruction::BinaryOps::FDiv,
        { Instruction::BinaryOps::FMul, Instruction::BinaryOps::FDiv } }
    };

bool LoopCarriedSCC::classof(const GenericSCC *s) {
  return (s->getKind() >= GenericSCC::SCCKind::LOOP_CARRIED)
         && (s->getKind() <= GenericSCC::SCCKind::LAST_LOOP_CARRIED);
}

} // namespace llvm::noelle
