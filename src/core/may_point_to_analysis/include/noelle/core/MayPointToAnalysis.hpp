#pragma once

#include "noelle/core/Utils.hpp"
#include "noelle/core/CallGraph.hpp"

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

using Variables = std::unordered_set<Variable *>;
using MemoryObjects = std::unordered_set<MemoryObject *>;
using Pointers = std::unordered_set<Pointer *>;

class PointToGraph {
public:
  PointToGraph();
  MemoryObjects getPointees(Pointer *pointer);
  bool setPointees(Pointer *pointer, MemoryObjects newPtes);
  bool addPointees(Pointer *pointer, MemoryObjects newPtes);
  MemoryObject *mustPointToMemory(Pointer *pointer);
  MemoryObjects getReachableMemoryObjects(Pointer *pointer);
  Pointers getAllPointers(void);

private:
  std::unordered_map<Pointer *, MemoryObjects> ptGraph;
};

class FunctionSummary {
public:
  FunctionSummary(Function *currentF);
  ~FunctionSummary();
  /*
   * Memory objects that can be accessed after the function returns.
   * i.e. memory objects reachable form
   * 1. global variables,
   * 2. unknown memory object,
   * 3. input arguments,
   * 4. return values.
   */
  MemoryObjects memoryObjectsCanBeAccessedAfterReturn();

  Function *currentF;

  std::unordered_set<CallBase *> mallocInsts;
  std::unordered_set<CallBase *> callocInsts;
  std::unordered_set<CallBase *> reallocInsts;
  std::unordered_set<CallBase *> freeInsts;
  std::unordered_set<CallBase *> callInsts;
  std::unordered_set<AllocaInst *> allocaInsts;
  std::unordered_set<LoadInst *> loadInsts;
  std::unordered_set<StoreInst *> storeInsts;
  std::unordered_set<ReturnInst *> returnInsts;

  PointToGraph *functionPointToGraph;

  /*
   * Memory objects directly pointed by all return values of the function
   * For example, the functions has two returns: `return %1` and `return %2`,
   * and %1 points to M1 and M2, %2 points to M3 and M4, M1 points to M5
   * then returnMemoryObjects = { M1, M2, M3, M4 }, not including M5
   */
  MemoryObjects returnMemoryObjects;

  MemoryObjects mustHeapMemoryObjects;
};

/*
 * PointToSummary will do a may-point-to-analysis, then stores the point-to
 * information The may-point-to-analysis is an forward, intra-procedural, may
 * analysis
 */
class PointToSummary {
public:
  PointToSummary(Module &M);
  ~PointToSummary();

  Variable *getVariable(Value *source);
  MemoryObject *getMemoryObject(Value *source);
  FunctionSummary *getFunctionSummary(Function *function);

  Module &M;

  /*
   * Memory objects allocated by global variable declarations
   * For example, in the global variable declaration `@gv = dso_local global
   * [256 x i8]` the `dso_local global [256 x i8]` is one global memory object
   */
  MemoryObjects globalMemoryObjects;
  MemoryObject *unknownMemoryObject;
  std::unordered_map<Function *, FunctionSummary *> functionSummaries;

private:
  std::unordered_map<Value *, Variable *> variables;
  std::unordered_map<Value *, MemoryObject *> memoryObjects;
};

class MayPointToAnalysis {
public:
  MayPointToAnalysis();
  PointToSummary *getPointToSummary(Module &M, CallGraph *pcf);
  // LiveMemorySummary *getLiveMemorySummary(FunctionSummary *funcSum);
  ~MayPointToAnalysis();

private:
  bool FS(FunctionSummary *funcSum,
          Instruction *inst,
          std::unordered_set<Function *> &visited);

  bool enterUserDefinedFunctionFromCallBase(
      Function *calleeFunc,
      CallBase *callInst,
      std::unordered_set<Function *> &visited);

  bool enterUnknownExternalFunctionFromCallBase(CallBase *callInst);

  std::unordered_set<Function *> getPossibleCallees(CallBase *callInst);

  void updateFunctionSummaryUntilFixedPoint(
      Function *currentF,
      std::unordered_set<Function *> &visited);

  CallGraph *pcf;
  PointToSummary *ptSum;
};

} // namespace llvm::noelle