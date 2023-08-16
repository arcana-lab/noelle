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
#pragma once

#include "noelle/core/Utils.hpp"

namespace llvm::noelle {

typedef uint64_t NodeID;

class MpaSummary {
public:
  MpaSummary(Function *currentF);

  Function *currentF;

  std::unordered_set<StoreInst *> storeInsts;
  std::unordered_set<AllocaInst *> allocaInsts;
  std::unordered_set<CallBase *> mallocInsts;
  std::unordered_set<CallBase *> callocInsts;
  std::unordered_set<CallBase *> freeInsts;

  bool mayBePointedByUnknown(Value *ptr);
  std::unordered_set<Value *> getPointees(Value *ptr);

private:
  /*
   * All pointers may be used as return value of current function.
   */
  std::unordered_set<Value *> returnPointers;
  /*
   * All pointers in current function.
   */
  std::unordered_set<Value *> pointers;

  bool mpaFinished = false;
  const NodeID UnknownMemobjId = 0;
  NodeID nextNodeId = 1;

  /*
   * Assign node id to each pointer in current function.
   */
  std::unordered_map<Value *, NodeID> ptr2nodeId;

  /*
   * Assign node id to each memory object, the memory object is represented
   * by the souce of allocation.
   *
   * 1. The allocation source can be a AllocaInst, a malloc/calloc call,
   *    or allocaCandidate.
   * 2. Besides, we have a "unknown" memobj, which always has node id 0 and
   *    its allocation souce is nullptr, which is a summary of all memobjs not
   *    allocated by current function.
   *
   * Arguments of current function and global variables point to "unknown"
   * memobj. Pointers returned by callInsts point to "unknown" memobj. "unkonwn"
   * memobj points to itself since it's a summary.
   */
  std::unordered_map<Value *, NodeID> memobj2nodeId;
  std::unordered_map<NodeID, Value *> nodeId2memobj;

  /*
   * The memory objects pointed by pointers and memory objects.
   */
  std::unordered_map<NodeID, BitVector> pointsTo;

  /*
   * A copy edge (src => dest) means that dest may point to the same memobjs
   * as src. To be more specific, pts(dest) = pts(dest) U pts(src).
   * Here both src and dest can be pointers or memobjs.
   *
   * For example. if we have %1 = select i1 %cond, i32* %ptr1, i32* %ptr2,
   * then we add two copy edges: "%ptr1 => %1" and "%ptr2 => %1",
   * and we have pts(%1) = pts(%ptr1) U pts(%ptr2).
   */
  std::unordered_map<NodeID, std::unordered_set<NodeID>> copyOutEdges;

  /*
   * storeInst = `store i32* %p1, i32** %p2` is an incomingStore of %p2
   * since %p2 is used as the pointer operand of storeInst, and the points-to
   * info flows from %p1 into the memory objects pointed by %p2.
   */
  std::unordered_map<NodeID, std::unordered_set<StoreInst *>> incomingStores;

  /*
   * `%3 = load i32*, i32** %p2` is an outgoingLoad of %p2 since %p2 is used
   * as the pointer operand of loadInst, the points-to info flows out of the
   * memobjs pointed by %2 to the loadInst.
   */
  std::unordered_map<NodeID, std::unordered_set<LoadInst *>> outgoingLoads;

  /*
   * All pointers used as arguments of callInsts in current function.
   */
  std::unordered_set<NodeID> usedAsFuncArg;

  /*
   * allocaCandidate is a global variable that we want to privatize into
   * current function, i.e. it works like an allocaInst in current function.
   *
   * Usually, global variables point to "unknown" memobj, but allocaCandidate
   * is an exception. We assign a node id for the memobj of allocaCandidate,
   * which is different from all other memobjs from alloca/malloc/calloc and
   * "unkonwn" memobj.
   *
   * The reason to introduce allocaCandidate is to check the memory object of
   * the global variable will not be pointed by other global variables,
   * arguments of the current function, and the return values of the current
   * function. i.e. allocaCandidate and notPrivatizable() help check the third
   * condition from Privatizer::getPrivatizableFunctions().
   */
  GlobalVariable *allocaCandidate = nullptr;

  std::queue<NodeID> worklist;

  BitVector getEmptyBitVector(void);
  BitVector onlyPointsTo(NodeID memobjId);
  std::unordered_set<Value *> getAllocations(void);
  NodeID getPtrId(Value *v);
  bool addCopyEdge(NodeID src, NodeID dst);

  void mayPointsToAnalysis(void);
  void initPtInfo(void);
  void solveWorklist(void);

  void handleLoadStore(NodeID ptrId);
  void handleFuncUsers(NodeID ptrId);
  void handleCopyEdges(NodeID srcId);

  BitVector getPointees(NodeID nodeId);
  std::unordered_set<NodeID> getReachableMemobjs(NodeID ptrId);
  bool unionPts(NodeID srcId, NodeID dstId);
};

class MayPointsToAnalysis {
public:
  MayPointsToAnalysis();

  bool mayAlias(Value *ptr1, Value *ptr2);
  bool mayEscape(Instruction *inst);
  bool notPrivatizable(GlobalVariable *globalVar, Function *currentF);

  ~MayPointsToAnalysis();

private:
  std::unordered_map<Function *, MpaSummary *> functionSummaries;

  MpaSummary *getFunctionSummary(Function *currentF);
};

} // namespace llvm::noelle
