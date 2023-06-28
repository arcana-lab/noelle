#pragma once

#include "noelle/core/Utils.hpp"
#include "noelle/core/CallGraph.hpp"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

#include <map>
#include <set>
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
  Value *getSource(void);
  virtual PointNodeType getType(void) = 0;

private:
  Value *source;
};

class Variable : public Pointer {
public:
  Variable(Value *source);
  PointNodeType getType(void) override;
};

class MemoryObject : public Pointer {
public:
  MemoryObject(Value *source);
  PointNodeType getType(void) override;
};

using FunctionCall = std::pair<Function *, Function *>;
using Variables = std::set<Variable *>;
using MemoryObjects = std::set<MemoryObject *>;

class PointToGraph {
public:
  PointToGraph();
  MemoryObjects getPointees(Pointer *pointer);
  bool setPointees(Pointer *pointer, MemoryObjects pointees);
  MemoryObject *mustPointToMemory(Pointer *pointer);
  MemoryObjects reachableMemoryObjects(Pointer *pointer);

private:
  std::map<Pointer *, MemoryObjects> ptGraph;
};

class FunctionSummary {
public:
  FunctionSummary(Function *caller, Function *currentF);
  ~FunctionSummary();

  Function *currentF;
  Function *caller;

  std::set<CallBase *> mallocInsts;
  std::set<CallBase *> callocInsts;
  std::set<CallBase *> reallocInsts;
  std::set<CallBase *> freeInsts;
  std::set<CallBase *> unknownFuntctionCalls;
  std::set<AllocaInst *> allocaInsts;
  std::set<LoadInst *> loadInsts;
  std::set<StoreInst *> storeInsts;
  std::set<ReturnInst *> returnInsts;

  PointToGraph *functionPointToGraph;
  /*
   * Record all memory objects that escape from the current function.
   * An escaped memory object could be read or written after the current
   * function returns. Therefore, we could not turn it to allocaInst.
   */
  MemoryObjects canBeAccessedAfterReturn;

  MemoryObjects returnValue;

  MemoryObjects mustHeap;
};

/*
 * PointToSummary will do a may-point-to-analysis, then stores the point-to
 * information The may-point-to-analysis is an forward, intra-procedural, may
 * analysis
 */
class PointToSummary {
public:
  PointToSummary(Module &M, CallGraph *callGraph);
  ~PointToSummary();

  Variable *getVariable(Value *source);
  MemoryObject *getMemoryObject(Value *source);

  Module &M;
  std::map<FunctionCall, FunctionSummary *> funcSums;

  /*
   * Memory objects allocated by global variable declarations
   * For example, in the global variable declaration `@gv = dso_local global
   * [256 x i8]` the `dso_local global [256 x i8]` is one global memory object
   */
  MemoryObjects globalMemoryObjects;

private:
  std::map<Value *, Variable *> variables;
  std::map<Value *, MemoryObject *> memoryObjects;
};

class LiveMemorySummary {
public:
  std::set<CallBase *> allocable;
  std::set<CallBase *> removable;
};

class MayPointToAnalysis {
public:
  MayPointToAnalysis();
  PointToSummary *getPointToSummary(Module &M, CallGraph *callGraph);
  // LiveMemorySummary *getLiveMemorySummary(FunctionSummary *funcSum);
  ~MayPointToAnalysis();

private:
  bool FS(FunctionSummary *funcSum,
          Instruction *inst,
          std::set<FunctionCall> &visited);
  void updateFunctionSummaryUntilFixedPoint(FunctionCall functionCall,
                                            std::set<FunctionCall> &visited);

  PointToSummary *ptSum;
};

std::string getCalledFuncName(CallBase *callInst);

} // namespace llvm::noelle