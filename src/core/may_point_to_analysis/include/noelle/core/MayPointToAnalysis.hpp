#pragma once

#include "noelle/core/Utils.hpp"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace llvm::noelle {

/*
 * PointNodeType is used to represent point-to information
 * Following the semantics of LLVM IR,
 * a pointer can either be a variable or a memory object,
 * while the pointee can only be a memory object.
 *
 * For example, in the inst: %1 = tail call i8* @malloc(i64 8)
 * `%1` is the variable, `the memory object M1 allocated by malloc` is the
 * pointee. we have a mapping: %1 -> { M1 }
 *
 * For a global variable: @gv = dso_local global [256 x i8]
 * `@gv` is the variable, `dso_local global [256 x i8]` is the pointee memory
 * object. we have a mapping: @gv -> { dso_local global [256 x i8] }
 */
enum PointNodeType { VARIABLE, MEMORY_OBJECT };

/*
 * For the inst: %1 = tail call i8* @malloc(i64 8)
 * "%1 is the variable" refers to `Variable(%1 = tail call i8* @malloc(i64 8))`
 * "the memory object M1 allocated by malloc" refers to `MemoryObject(%1 = tail
 * call i8* @malloc(i64 8))`
 *
 * `Variable::source` and `MemoryObject::source` refers the original
 * instruction. In this case, it is `%1 = tail call i8* @malloc(i64 8)`.
 */
class Pointer {
public:
  Pointer(Value *source);
  Value *getSource();
  virtual PointNodeType getType() = 0;

private:
  Value *source;
};

class Variable : public Pointer {
public:
  Variable(Value *source);
  PointNodeType getType() override;
};

class MemoryObject : public Pointer {
public:
  MemoryObject(Value *source, bool prev = false);
  PointNodeType getType() override;

private:
  bool prevLoopAllocated = false;
};

using Variables = std::unordered_set<Variable *>;
using MemoryObjects = std::unordered_set<MemoryObject *>;
using PointToGraph = std::unordered_map<Pointer *, MemoryObjects>;

class FunctionSummary {
public:
  FunctionSummary(Function *F);

  Module *M;
  Function *F;

  uint64_t basicBlockCount = 0;
  std::unordered_set<CallBase *> mallocInsts;
  std::unordered_set<CallBase *> callocInsts;
  std::unordered_set<CallBase *> reallocInsts;
  std::unordered_set<CallBase *> freeInsts;
  std::unordered_set<CallBase *> unknownFuntctionCalls;
  std::unordered_set<AllocaInst *> allocaInsts;
  std::unordered_set<LoadInst *> loadInsts;
  std::unordered_set<StoreInst *> storeInsts;
};

/*
 * PointToSummary will do a may-point-to-analysis, then stores the point-to
 * information The may-point-to-analysis is an forward, intra-procedural, may
 * analysis
 */
class PointToSummary {
public:
  PointToSummary(FunctionSummary *funSum);
  ~PointToSummary();

  MemoryObjects getPointees(PointToGraph &ptGraph, Pointer *pointer);
  MemoryObjects getPointees(PointToGraph &ptGraph, Value *pointer);
  MemoryObject *mustPointToMemory(PointToGraph &ptGraph, Pointer *pointer);
  MemoryObjects reachableMemoryObjects(PointToGraph &ptGraph, Pointer *pointer);
  Variable *getVariable(Value *source);
  MemoryObject *getMemoryObject(Value *source);

  Module *M;
  std::unordered_map<Instruction *, PointToGraph> instIN;
  std::unordered_map<Instruction *, PointToGraph> instOUT;
  std::unordered_map<BasicBlock *, PointToGraph> bbIN;
  std::unordered_map<BasicBlock *, PointToGraph> bbOUT;

  /*
   * Assume we have inst `%1 = tail call i8* @malloc(i64 8)` in loop, which will
   * be executed many times. then we have M1 and M1_prev, where M1 is the memory
   * object allocated in the latest loop, while M1_prev is the memory object
   * allocated in any previous loop. prevLoopAllocated is used to record the
   * mapping: M1 -> M1_prev
   *
   * Here lastest means the last execution of `%1 = tail call i8* @malloc(i64
   * 8)` if current iteration is iteration 3 (start from 0), M1 is the memory
   * object allocated by the 4th execution of `%1 = tail call i8* @malloc(i64
   * 8)` M1_prev refers to the memory objects allocated by the 1st, 2nd, 3rf
   * execution of `%1 = tail call i8* @malloc(i64 8)`
   *
   */
  std::unordered_map<MemoryObject *, MemoryObject *> prevLoopAllocated;

  /*
   * if only M1 is read or written (see checkAmbiguity() in
   * MayPointToAnalysis.cpp), then we could say that the memory object allocated
   * by `%1 = tail call i8* @malloc(i64 8)` will only be read and written in the
   * current iteration, therefore we could safely hoist the malloc to the entry
   * block, and turn it to allocainst.
   *
   * if some M1_prev is read or written, then we could not hoist the malloc to
   * the entry block. can the corresponding M1 is marked as ambiguous.
   */
  MemoryObjects ambiguous;
  /*
   * Memory objects allocated by global variable declarations
   * For example, in the global variable declaration `@gv = dso_local global
   * [256 x i8]` the `dso_local global [256 x i8]` is one global memory object
   */
  MemoryObjects globalMemoryObjects;
  /*
   * Becauase of some bugs of LLVM, if allocaInst is the destination of the
   * MemCpyInst, The copy will not change the value of the destination.
   * Therefore, if some `%1 = tail call i8* @malloc(i64 8)` used as the
   * destination of MemCpyInst, Then we could not turn it to allocaInst.
   */
  MemoryObjects mustHeap;
  /*
   * Record all memory objects that escape from the current function.
   * An escaped memory object could be read or written after the current
   * function returns. Therefore, we could not turn it to allocaInst.
   */
  MemoryObjects escaped;
  /*
   * nonLocalMemoryObject is summary for memory objects allocated by other
   * functions. Being pointed by nonLocalMemoryObject means a memory object
   * escaped.
   */
  MemoryObject *nonLocalMemoryObject;

private:
  std::unordered_map<Value *, Variable *> variables;
  std::unordered_map<Value *, MemoryObject *> memoryObjects;
};

class LiveMemorySummary {
public:
  std::unordered_set<CallBase *> allocable;
  std::unordered_set<CallBase *> removable;
};

class MayPointToAnalysis {
public:
  MayPointToAnalysis(Function *F);
  FunctionSummary *getFunctionSummary();
  PointToSummary *getPointToSummary();
  LiveMemorySummary *getLiveMemorySummary();
  ~MayPointToAnalysis();

private:
  FunctionSummary *functionSummary;
  PointToSummary *pointToSummary;
  PointToGraph mergeAllPredOut(BasicBlock *bb, PointToSummary *ptSum);
  PointToGraph FS(Instruction *inst, PointToSummary *ptSum);
};

std::string getCalledFuncName(CallBase *callInst);

} // namespace llvm::noelle