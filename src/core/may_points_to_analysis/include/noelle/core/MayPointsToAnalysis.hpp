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
#ifndef NOELLE_SRC_CORE_MAY_POINTS_TO_ANALYSIS_MAYPOINTSTOANALYSIS_H_
#define NOELLE_SRC_CORE_MAY_POINTS_TO_ANALYSIS_MAYPOINTSTOANALYSIS_H_

#include "noelle/core/Utils.hpp"

namespace arcana::noelle {

typedef uint32_t NodeID;

class MpaSummary {
public:
  MpaSummary(Function *currentF);

  Function *currentF;

  std::unordered_set<StoreInst *> storeInsts;
  std::unordered_set<AllocaInst *> allocaInsts;
  std::unordered_set<CallBase *> mallocInsts;
  std::unordered_set<CallBase *> callocInsts;
  std::unordered_set<CallBase *> freeInsts;

  bool mayBePointedByUnknown(Value *memobj);
  bool mayBePointedByReturnValue(Value *memobj);
  std::unordered_set<Value *> getPointeeMemobjs(Value *ptr);

  void doMayPointsToAnalysis(void);
  void doMayPointsToAnalysisFor(GlobalVariable *globalVar);
  void clearPointsToSummary(void);

private:
  /*
   * All pointers may be used as return value of the current function.
   */
  std::unordered_set<Value *> returnPointers;
  /*
   * All pointers in the current function.
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
   * Assign node id to each memory object,
   *
   * 1. The memory object is represented by (1) the AllocaInst or malloc/calloc
   * instrucions in the current function that allocates it, or (2) the
   * privatizeCandidate.
   *
   * 2. Besides, we have a "unknown" memory object, which is a summary of all
   * memory objects not allocated in the current function. The "unknown" memory
   * object always has NodeId 0.
   *
   * To preserve conservativeness, arguments of the current function and global
   * variables always point to the "unknown" memory object because they point to
   * some memory objects that are not allocated in the current funciton.
   *
   * Besides, pointers returned by callInsts point to the "unknown" memory
   * object because they may also point to some memory objects that are not
   * allocated in the current funciton.
   *
   * The "unkonwn" memory object is a summary of several memory objects, hence
   * it will point to itself.
   */
  std::unordered_map<Value *, NodeID> memobj2nodeId;
  std::unordered_map<NodeID, Value *> nodeId2memobj;

  /*
   * The points-to graph.
   * The key of the points-to graph is a NodeID that reresents one pointer or
   * memory object. The value is a bitvector containing the NodeIDs of the
   * pointee memory objects.
   */
  std::unordered_map<NodeID, BitVector> pointsTo;

  /*
   * A copy edge (src => dest) means that dest may point to the same memory
   * objects as src. To be more specific, pts(dest) = pts(dest) U pts(src). Here
   * both src and dest can be pointers or memory objects.
   *
   * For example. if we have `%1 = select i1 %cond, i32* %ptr1, i32* %ptr2`,
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
   * All pointers used as arguments of callInsts in the current function.
   */
  std::unordered_set<NodeID> usedAsFuncArg;

  /*
   * privatizeCandidate is a global variable that we want to privatize into the
   * current function. "Privatize" means we want to transform the global
   * variable to an AllocaInst in the current function.
   *
   * We use MayPointsToAnalysis::notPrivatizable() to check whether
   * privatizeCandidate can be privatized into the current function. The idea of
   * notPrivatizable() is described as follows.
   *
   * If we want to privatize privatizeCandidate, we must ensure other functions
   * will not access the memory object of privatizeCandidate through pointers.
   * To put it another way, if privatizeCandidate is privatized to an
   * AllocaInst, the AllocaInst shall not escape.
   *
   * Usually, the memory object of one global variable is represented by the
   * "unknown" memory object. Such a conservative strategy will lose the
   * points-to info of the memory object of privatizeCandidate.
   *
   * To handle this issue, we must treat privatizeCandidate in a different way
   * from other global variables. We assign a non-zero NodeID to represent the
   * memory object of privatizeCandidate, just like it's an AllocaInst.
   *
   * If this NodeID may be pointed directly or indirectly by the "unknown"
   * memory object, or the return value of the current function, then we know if
   * privatizeCandidate is privatized to an AllocaInst, this AllocaInst may
   * escape and be accessed after the current function returns.
   *
   * In such a case, privatizing privatizeCandidate may cause undefined
   * behavior, we should give up the privatization and notPrivatizable() will
   * return true. Otherwise, notPrivatizable() will return false.
   *
   * Note notPrivatizable() returning true means we should give up the
   * privatization, while notPrivatizable() returning false only means the
   * AllocaInst transfromed from privatizeCandidate will not escape. The false
   * reult doesn't mean it's safe to privatize the global variable because we
   * must check more things.
   */
  GlobalVariable *privatizeCandidate = nullptr;

  std::queue<NodeID> worklist;

  BitVector getEmptyBitVector(void);
  BitVector onlyPointsTo(NodeID memobjId);
  std::unordered_set<Value *> getAllocations(void);
  NodeID getPtrId(Value *v);
  bool addCopyEdge(NodeID src, NodeID dst);

  void initPtInfo(void);
  void solveWorklist(void);

  void handleLoadStore(NodeID ptrId);
  void handleFuncUsers(NodeID ptrId);
  void handleCopyEdges(NodeID srcId);

  BitVector getPointeeBitVector(NodeID nodeId);
  std::unordered_set<NodeID> getreachableMemobjIds(NodeID ptrId);
  bool unionPts(NodeID srcId, NodeID dstId);
};

class MayPointsToAnalysis {
public:
  MayPointsToAnalysis();

  bool mayAlias(Value *ptr1, Value *ptr2);
  bool mayEscape(Instruction *inst);
  bool notPrivatizable(GlobalVariable *globalVar, Function *currentF);
  std::unordered_set<Value *> getPointees(Value *ptr, Function *currentF);

  ~MayPointsToAnalysis();

private:
  std::unordered_map<Function *, MpaSummary *> functionSummaries;

  MpaSummary *getFunctionSummary(Function *currentF);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_MAY_POINTS_TO_ANALYSIS_MAYPOINTSTOANALYSIS_H_
