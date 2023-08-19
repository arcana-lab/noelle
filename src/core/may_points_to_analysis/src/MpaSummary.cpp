/*
 * Copyright 2023 Xiao Chen
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
#include "noelle/core/MayPointsToAnalysis.hpp"
#include "MpaUtils.hpp"

using namespace std;

namespace llvm::noelle {

MpaSummary::MpaSummary(Function *currentF) : currentF(currentF) {

  auto insertPointer = [&](Value *v) {
    if (v->getType()->isPointerTy()) {
      queue<Value *> todolist;
      todolist.push(v);

      while (!todolist.empty()) {
        auto ptr = todolist.front();
        todolist.pop();
        pointers.insert(ptr);

        if (isa<GetElementPtrInst>(ptr)) {
          auto gepInst = dyn_cast<GetElementPtrInst>(ptr);
          todolist.push(gepInst->getPointerOperand());
        } else if (isa<GEPOperator>(ptr)) {
          auto gepOp = dyn_cast<GEPOperator>(ptr);
          todolist.push(gepOp->getPointerOperand());
        } else if (isa<BitCastInst>(ptr) || isa<BitCastOperator>(ptr)) {
          todolist.push(ptr->stripPointerCasts());
        }
      }
    }
  };

  /*
   * 1. Collect store/alloca/malloc/calloc/free insts in the function.
   * 2. Collect all pointers in the function.
   * 3. Collect all pointers may be returned by the function.
   */

  for (auto &arg : currentF->args()) {
    insertPointer(&arg);
  }

  for (auto &bb : *currentF) {
    for (auto &inst : bb) {
      if (isa<LoadInst>(&inst)) {
        auto loadInst = dyn_cast<LoadInst>(&inst);
        insertPointer(loadInst->getPointerOperand());
        insertPointer(loadInst);
      } else if (isa<StoreInst>(&inst)) {
        auto storeInst = dyn_cast<StoreInst>(&inst);
        storeInsts.insert(storeInst);
        insertPointer(storeInst->getValueOperand());
        insertPointer(storeInst->getPointerOperand());
      } else if (isa<BitCastInst>(&inst) || isa<GetElementPtrInst>(&inst)) {
        insertPointer(&inst);
      } else if (isa<AllocaInst>(&inst)) {
        auto allocaInst = dyn_cast<AllocaInst>(&inst);
        allocaInsts.insert(allocaInst);
        insertPointer(allocaInst);
      } else if (isa<CallBase>(&inst)) {
        auto callInst = dyn_cast<CallBase>(&inst);
        auto calleeType = getCalleeFunctionType(callInst);
        switch (calleeType) {
          case MALLOC:
            mallocInsts.insert(callInst);
            insertPointer(callInst);
            break;
          case CALLOC:
            callocInsts.insert(callInst);
            insertPointer(callInst);
            break;
          case FREE:
            freeInsts.insert(callInst);
            break;
          case REALLOC:
            insertPointer(callInst);
            break;
          default:
            insertPointer(callInst);
            for (auto &arg : callInst->arg_operands()) {
              auto operand = callInst->getArgOperand(arg.getOperandNo());
              insertPointer(operand);
            }
            break;
        }
      } else if (isa<PHINode>(&inst) && inst.getType()->isPointerTy()) {
        auto phiNode = dyn_cast<PHINode>(&inst);
        insertPointer(phiNode);
        for (auto &incoming : phiNode->incoming_values()) {
          insertPointer(incoming);
        }
      } else if (isa<SelectInst>(&inst) && inst.getType()->isPointerTy()) {
        auto selectInst = dyn_cast<SelectInst>(&inst);
        insertPointer(selectInst);
        insertPointer(selectInst->getTrueValue());
        insertPointer(selectInst->getFalseValue());
      } else if (isa<ReturnInst>(&inst)) {
        auto returnInst = dyn_cast<ReturnInst>(&inst);
        auto retVal = returnInst->getReturnValue();
        if (retVal && retVal->getType()->isPointerTy()) {
          returnPointers.insert(retVal);
        }
      }
    }
  }
}

unordered_set<Value *> MpaSummary::getPointeeMemobjs(Value *ptr) {
  assert(mpaFinished);

  auto stripped = strip(ptr);
  assert(ptr2nodeId.find(stripped) != ptr2nodeId.end());

  auto ptrId = ptr2nodeId[stripped];
  unordered_set<Value *> pointees;

  auto pointeeBitVec = getPointeeBitVector(ptrId);
  for (auto memobjId : pointeeBitVec.set_bits()) {
    if (memobjId == UnknownMemobjId) {
      pointees.insert(nullptr);
    } else {
      pointees.insert(nodeId2memobj[memobjId]);
    }
  }
  return pointees;
}

bool MpaSummary::mayBePointedByUnknown(Value *memobj) {
  assert(mpaFinished);
  assert(getAllocations().count(memobj) > 0);
  auto memobjId = memobj2nodeId.at(memobj);

  auto unknownPts = getreachableMemobjIds(UnknownMemobjId);
  if (unknownPts.find(memobjId) != unknownPts.end()) {
    return true;
  }
  return false;
}

bool MpaSummary::mayBePointedByReturnValue(Value *memobj) {
  assert(mpaFinished);
  assert(getAllocations().count(memobj) > 0);
  auto memobjId = memobj2nodeId.at(memobj);

  for (auto retPtr : returnPointers) {
    auto retPtrId = getPtrId(retPtr);
    auto retMemobjs = getreachableMemobjIds(retPtrId);
    if (retMemobjs.find(memobjId) != retMemobjs.end()) {
      return true;
    }
  }
  return false;
}

BitVector MpaSummary::getPointeeBitVector(NodeID nodeId) {
  if (pointsTo.find(nodeId) != pointsTo.end()) {
    return pointsTo[nodeId];
  } else {
    return getEmptyBitVector();
  }
}

unordered_set<NodeID> MpaSummary::getreachableMemobjIds(NodeID ptrId) {
  unordered_set<NodeID> reachable;
  queue<NodeID> todolist;
  todolist.push(ptrId);

  while (!todolist.empty()) {
    auto nodeId = todolist.front();
    todolist.pop();

    auto pointeeBitVec = getPointeeBitVector(nodeId);
    for (auto memObj : pointeeBitVec.set_bits()) {
      if (reachable.find(memObj) == reachable.end()) {
        reachable.insert(memObj);
        todolist.push(memObj);
      }
    }
  }
  return reachable;
}

BitVector MpaSummary::getEmptyBitVector(void) {
  auto bitVecSize = 1 + getAllocations().size();
  return BitVector(bitVecSize, false);
}

BitVector MpaSummary::onlyPointsTo(NodeID memobjId) {
  auto emptyPts = getEmptyBitVector();
  assert(memobjId >= 0 && memobjId < emptyPts.size());
  emptyPts.set(memobjId);
  return emptyPts;
}

unordered_set<Value *> MpaSummary::getAllocations(void) {
  unordered_set<Value *> allocations;
  for (auto &allocaInst : allocaInsts) {
    allocations.insert(allocaInst);
  }
  for (auto &mallocInst : mallocInsts) {
    allocations.insert(mallocInst);
  }
  for (auto &callocInst : callocInsts) {
    allocations.insert(callocInst);
  }
  if (privatizeCandidate) {
    allocations.insert(privatizeCandidate);
  }
  return allocations;
}

NodeID MpaSummary::getPtrId(Value *v) {
  assert(v->getType()->isPointerTy());

  auto stripped = strip(v);
  if (ptr2nodeId.find(stripped) == ptr2nodeId.end()) {
    ptr2nodeId[stripped] = nextNodeId++;
  }
  return ptr2nodeId[stripped];
}

bool MpaSummary::addCopyEdge(NodeID src, NodeID dst) {
  return copyOutEdges[src].insert(dst).second;
}

void MpaSummary::doMayPointsToAnalysis(void) {
  if (!mpaFinished) {
    initPtInfo();
    solveWorklist();
    mpaFinished = true;
  }
}

void MpaSummary::doMayPointsToAnalysisFor(GlobalVariable *globalVar) {
  clearPointsToSummary();
  privatizeCandidate = globalVar;
  doMayPointsToAnalysis();
}

void MpaSummary::clearPointsToSummary(void) {
  privatizeCandidate = nullptr;
  mpaFinished = false;
  nextNodeId = 1;
  ptr2nodeId.clear();
  memobj2nodeId.clear();
  nodeId2memobj.clear();
  pointsTo.clear();
  copyOutEdges.clear();
  incomingStores.clear();
  outgoingLoads.clear();
  usedAsFuncArg.clear();
}

void MpaSummary::initPtInfo(void) {

  auto allocations = getAllocations();

  /*
   * Assign NodeIDs to memory objects
   * 1. The "unknown" memory object always had NodeID = 0 (see
   * MayPointsToAnalysis.hpp).
   * 2. For each memory object allocated by alloca/malloc/calloc, assign a
   * unique NodeID.
   * 3. If a global variable is privatizeCandidate, its memory objecg will also
   * be assigned a unique NodeID.
   */
  nodeId2memobj[UnknownMemobjId] = nullptr;
  memobj2nodeId[nullptr] = UnknownMemobjId;

  for (auto &memobj : allocations) {
    auto nodeId = nextNodeId++;
    nodeId2memobj[nodeId] = memobj;
    memobj2nodeId[memobj] = nodeId;
  }

  /*
   * 1. Assign NodeID to each pointer.
   * NOTE: The NodeID for the pointer is different from the NodeID for the
   * memobj. For example, if we have `%1 = alloca i32`, the NodeID for the
   * allocated memory object may be 1, the NodeID for the pointer %1 may be 4.
   *
   * 2. Initialize points-to graph.
   * (1) The "unknown" memory object conservatively points to itself since it's
   *     a summary.
   * (2) Pointers of alloca/malloc/calloc point to the allocated memobj.
   * (3) Arguments of the current function will point to the "unknown" memory
   *     object, because arguments will points to memory objects not allocated
   *     in the current function.
   * (4) Similarly, global variables and the callInsts will also point to the
   *     "unknown" memory object.
   * (5) PrivatizeCandidate will points to its own memory object instead of the
   *     "unknown" memory object (see MayPointsToAnalysis.hpp).
   *
   * 3. Add copy edges for pointers and memory objects.
   * (1) Copy edges between pointers can be added through PHINode, SelectInst,
   *     MemcpyInst and realloc().
   * (2) Add copy edges from arguments of callInsts to the "unknown" memory
   * object because the "unknown" memory object may point to escaped memory
   * objects. (3) Add copy edges from the "unknown" memory object to the
   * callInst if the callInst returns a pointer.
   *
   * 4. Record uses of pointers.
   * If a pointer is used as the pointer operand of a store/load instruction,
   * or it is used as an argument of a callInst, record these uses since they
   * can help the may points-to analysis add more copy edges.
   */
  pointsTo[UnknownMemobjId] = onlyPointsTo(UnknownMemobjId);

  for (auto &ptr : pointers) {
    auto ptrId = getPtrId(ptr);

    if (allocations.find(ptr) != allocations.end()) {
      auto memobjId = memobj2nodeId[ptr];
      pointsTo[ptrId] = onlyPointsTo(memobjId);
    } else if (isa<PHINode>(ptr)) {
      auto phiNode = dyn_cast<PHINode>(ptr);
      for (auto &incoming : phiNode->incoming_values()) {
        auto incomingPtrId = getPtrId(incoming);
        addCopyEdge(incomingPtrId, ptrId);
      }
    } else if (isa<SelectInst>(ptr)) {
      auto selectInst = dyn_cast<SelectInst>(ptr);
      auto trueValuePtrId = getPtrId(selectInst->getTrueValue());
      auto falseValuePtrId = getPtrId(selectInst->getFalseValue());
      addCopyEdge(trueValuePtrId, ptrId);
      addCopyEdge(falseValuePtrId, ptrId);
    } else if (isa<Argument>(ptr) || isa<GlobalVariable>(ptr)) {
      pointsTo[ptrId] = onlyPointsTo(UnknownMemobjId);
    } else if (isa<CallBase>(ptr)) {
      auto callInst = dyn_cast<CallBase>(ptr);
      auto calleeType = getCalleeFunctionType(callInst);
      switch (calleeType) {
        case REALLOC:
          addCopyEdge(getPtrId(callInst->getArgOperand(0)), ptrId);
          break;
        case USER_DEFINED:
        case UNKNOWN:
          pointsTo[ptrId] = onlyPointsTo(UnknownMemobjId);
          addCopyEdge(UnknownMemobjId, ptrId);
          break;
        default:
          break;
      }
    } else if (isa<ConstantPointerNull>(ptr)) {
      pointsTo[ptrId] = getEmptyBitVector();
    }

    for (auto user : ptr->users()) {
      if (isa<StoreInst>(user)) {
        auto storeInst = dyn_cast<StoreInst>(user);
        auto valueOperand = storeInst->getValueOperand();
        if (ptr == storeInst->getPointerOperand()
            && valueOperand->getType()->isPointerTy()) {
          incomingStores[ptrId].insert(storeInst);
        }
      } else if (isa<LoadInst>(user)) {
        auto loadInst = dyn_cast<LoadInst>(user);
        if (ptr == loadInst->getPointerOperand()
            && loadInst->getType()->isPointerTy()) {
          outgoingLoads[ptrId].insert(loadInst);
        }
      } else if (isa<CallBase>(user)) {
        auto callInst = dyn_cast<CallBase>(user);
        auto calleeType = getCalleeFunctionType(callInst);
        switch (calleeType) {
          case MEM_COPY:
            addCopyEdge(getPtrId(callInst->getArgOperand(1)),
                        getPtrId(callInst->getArgOperand(0)));
            break;
          case USER_DEFINED:
          case UNKNOWN:
            usedAsFuncArg.insert(ptrId);
            addCopyEdge(ptrId, UnknownMemobjId);
          default:
            break;
        }
      }
    }
  }
}

void MpaSummary::solveWorklist(void) {
  worklist = {};
  for (auto &[ptr, ptrId] : ptr2nodeId) {
    worklist.push(ptrId);
  }

  while (!worklist.empty()) {
    auto nodeID = worklist.front();
    worklist.pop();
    handleLoadStore(nodeID);
    handleFuncUsers(nodeID);
    handleCopyEdges(nodeID);
  }
}

void MpaSummary::handleLoadStore(NodeID ptrId) {
  auto pointees = getPointeeBitVector(ptrId);
  for (auto memobjId : pointees.set_bits()) {
    /*
     * OutgoingLoads help us add new copy edges.
     *
     * For example, if pointers (%p2) and memobjs (@M1, @M2, @M3, @M4)
     * form this points-to graph:
     *
     * %p2 -> @M2, @M3; @M2 -> @M1; @M3 -> @M4,
     *
     * then `%3 = load i32*, i32** %p2` can add copy edges
     * "@M2 => %3" and "@M3 => %3"
     *
     * %3 will point to @M1 and @M4.
     */
    if (outgoingLoads.find(ptrId) != outgoingLoads.end()) {
      for (auto loadInst : outgoingLoads[ptrId]) {
        auto destId = getPtrId(loadInst);
        if (addCopyEdge(memobjId, destId)) {
          worklist.push(memobjId);
        }
      }
    }

    /*
     * IncomingStores help us add new copy edges.
     *
     * For example, if pointers (%p1, %p2) and memobjs (@M1, @M2, @M3)
     * form this points-to graph:
     *
     * %p1 -> @M1 ; %p2 -> @M2, @M3
     *
     * Then `store i32* %p1, i32** %p2` add copy edges "%p1 => @M2" and
     * "%p1 => @M3", and the points-to graph can be updated:
     *
     * ... ; @M2 -> @M1; @M3 -> @M1
     */
    if (incomingStores.find(ptrId) != incomingStores.end()) {
      for (auto storeInst : incomingStores[ptrId]) {
        auto srcId = getPtrId(storeInst->getValueOperand());
        if (addCopyEdge(srcId, memobjId)) {
          worklist.push(srcId);
        }
      }
    }
  }
}

void MpaSummary::handleFuncUsers(NodeID ptrId) {
  if (usedAsFuncArg.find(ptrId) == usedAsFuncArg.end()) {
    return;
  }
  /*
   * If a pointer is used as argument of a callInst, then all memory objects
   * pointed directly or indirectly by this pointer escape. To preserve
   * conservativeness, the "unknown" memory object and all escaped memory
   * objects point to each other.
   *
   *
   * (1) Add copy edges between the "unknown" memory object and all escaped
   * memory objects.
   *
   * If memobj @M2 escaped and its points-to info updated:
   *     pts(@M2) = ... U { @M1 },
   * such an update should be propagated because @M1 escaped too:
   *     pts("unknown") = ... U { @M1 }.
   *
   *
   * (2) Add copy edge from the pointer argument to the "unknown" memory object.
   *
   * Assume pointer %p1 is used as argument of a callInst `call @g(%p1)`, and we
   * have this points-to graph:
   *
   *     %p1 -> @M1, @M2 ; @M1 -> @M3, @M4
   *
   * Adding copy edges between the "unknown" memory object and { @M1, @M2, @M3,
   * @M4 } is not enough because @M1 and @M2 are not pointed by any escaped
   * memory object. Hence the "unknown" memory object will not point to @M1 and
   * @M2.
   *
   * To solve this issue, we need also to add copy edges from %p1 to the
   * "unknown" memory object so that "unkonwn" can point to @M1 and @M2.
   *
   *
   * (3) Add copy edge from the "unknown" memory object to the return value.
   *
   * If the return value of the callInst is a pointer, it will conservatively
   * point to the "unknown" memory object and all escaped memory object.
   *
   *
   * Here we only handle case 1. Case 2 and 3 are already handled by
   * initPtInfo().
   */
  for (auto memobjId : getreachableMemobjIds(ptrId)) {
    bool changed = false;
    changed |= addCopyEdge(memobjId, UnknownMemobjId);
    changed |= addCopyEdge(UnknownMemobjId, memobjId);
    if (changed) {
      worklist.push(memobjId);
    }
  }
}

void MpaSummary::handleCopyEdges(NodeID srcId) {
  if (copyOutEdges.find(srcId) == copyOutEdges.end()) {
    return;
  }
  /*
   * Propogate the points-to info from srcId to destId through copy edges.
   * i.e. pts(destId) = pts(destId) U pts(srcId).
   * If pts(destId) is changed, add destId to worklist.
   */
  for (auto &destId : copyOutEdges[srcId]) {
    if (unionPts(srcId, destId)) {
      worklist.push(destId);
    }
  }
}

bool MpaSummary::unionPts(NodeID srcId, NodeID dstId) {
  BitVector oldPts = pointsTo[dstId];
  BitVector newPts = unite(oldPts, pointsTo[srcId]);
  if (oldPts == newPts) {
    return false;
  } else {
    pointsTo[dstId] = newPts;
    return true;
  }
}

} // namespace llvm::noelle
