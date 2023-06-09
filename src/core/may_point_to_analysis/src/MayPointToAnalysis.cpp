/*
 * Copyright 2019 - 2020 Angelo Matni, Simone Campanoni
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

#include "noelle/core/MayPointToAnalysis.hpp"
#include "noelle/core/MayPointToAnalysisUtils.hpp"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_set>

using namespace std;

namespace llvm::noelle {

const std::string MALLOC = "malloc";
const std::string CALLOC = "calloc";
const std::string REALLOC = "realloc";
const std::string FREE = "free";

const std::unordered_set<std::string> MEMORY_FUNCTIONS = { MALLOC,
                                                           CALLOC,
                                                           REALLOC,
                                                           FREE };
const std::unordered_set<std::string> READ_ONLY_LIB_FUNCTIONS = {
  "printf", "fprintf", "atoi",   "atol", "atoll",   "atof",
  "strtol", "strtoll", "strtod", "puts", "putchar", "fputc",
  "fputs",  "sqrt",    "rand",   "putc", "strncmp", "strlen"
};

Pointer::Pointer(Value *source) {
  this->source = source;
}

Value *Pointer::getSource() {
  return source;
}

Variable::Variable(Value *source) : Pointer(source) {}

PointNodeType Variable::getType() {
  return PointNodeType::VARIABLE;
}

MemoryObject::MemoryObject(Value *source, bool prev) : Pointer(source) {
  this->prevLoopAllocated = prev;
};

PointNodeType MemoryObject::getType() {
  return PointNodeType::MEMORY_OBJECT;
}

FunctionSummary::FunctionSummary(Function *F) {
  this->F = F;
  this->M = F->getParent();

  for (auto &bb : *F) {
    basicBlocks.insert(&bb);
    for (auto &inst : bb) {
      if (isa<AllocaInst>(inst)) {
        auto allocaInst = dyn_cast<AllocaInst>(&inst);
        allocaInsts.insert(allocaInst);
      } else if (isa<CallInst>(inst)) {
        auto callInst = dyn_cast<CallInst>(&inst);
        auto fname = getCalledFuncName(callInst);
        if (fname == MALLOC) {
          mallocInsts.insert(callInst);
        } else if (fname == CALLOC) {
          callocInsts.insert(callInst);
        } else if (fname == REALLOC) {
          reallocInsts.insert(callInst);
        } else if (fname == FREE) {
          freeInsts.insert(callInst);
        } else if (READ_ONLY_LIB_FUNCTIONS.count(fname) == 0) {
          unknownFuntctionCalls.insert(callInst);
        }
      } else if (isa<ReturnInst>(inst)) {
        auto retInst = dyn_cast<ReturnInst>(&inst);
        retInsts.insert(retInst);
      } else if (isa<LoadInst>(inst)) {
        auto loadInst = dyn_cast<LoadInst>(&inst);
        loadInsts.insert(loadInst);
      } else if (isa<StoreInst>(inst)) {
        auto storeInst = dyn_cast<StoreInst>(&inst);
        storeInsts.insert(storeInst);
      }
    }
  }
}

PointToSummary::PointToSummary(FunctionSummary *funcSum) {
  M = funcSum->M;

  for (auto &G : M->globals()) {
    auto globalVar = new Variable(&G);
    auto globalMemObj = new MemoryObject(&G);
    variables[&G] = globalVar;
    memoryObjects[&G] = globalMemObj;
    globalMemoryObjects.insert(globalMemObj);
  }

  for (auto bb : funcSum->basicBlocks) {
    bbIN[bb] = PointToGraph();
    bbOUT[bb] = PointToGraph();

    for (auto &inst : *bb) {
      instIN[&inst] = PointToGraph();
      instOUT[&inst] = PointToGraph();
    }
  }

  for (auto inst : funcSum->mallocInsts) {
    auto heapVar = new Variable(inst);
    auto heapMemObj = new MemoryObject(inst);
    variables[inst] = heapVar;
    memoryObjects[inst] = heapMemObj;
  }

  for (auto inst : funcSum->callocInsts) {
    auto heapVar = new Variable(inst);
    auto heapMemObj = new MemoryObject(inst);
    variables[inst] = heapVar;
    memoryObjects[inst] = heapMemObj;
  }

  for (auto inst : funcSum->allocaInsts) {
    auto stackVar = new Variable(inst);
    auto stackMemObj = new MemoryObject(inst);
    variables[inst] = stackVar;
    memoryObjects[inst] = stackMemObj;
  }

  for (auto &arg : funcSum->F->args()) {
    variables[&arg] = new Variable(&arg);
  }

  nonLocalMemoryObject = new MemoryObject(nullptr);
}

void PointToSummary::eraseDummyObjects() {
  for (auto &[_, variable] : variables) {
    free(variable);
  }
  for (auto &[_, memoryObject] : memoryObjects) {
    free(memoryObject);
  }
  for (auto &[_, memObjPrev] : prevLoopAllocated) {
    free(memObjPrev);
  }
  free(nonLocalMemoryObject);
}

/*
 * Returns the set of memory objects that the pointer may point to.
 * The pointer can be a variable or a memory object
 */
MemoryObjects PointToSummary::getPointees(PointToGraph &ptGraph,
                                          Pointer *pointer) {
  return (ptGraph.count(pointer) > 0) ? ptGraph[pointer] : MemoryObjects();
}

/*
 * Returns the set of memory objects that the pointer may point to.
 * Here the pointer is an instruction or global variable, but cannot be a memory
 * object. To get the getPointees of a memory object, the user must explicitly
 * create the memory object and use `getPointees(PointToGraph &ptGraph, Pointer
 * *ptr)`.
 */
MemoryObjects PointToSummary::getPointees(PointToGraph &ptGraph,
                                          Value *pointer) {
  return getPointees(ptGraph, getVariable(pointer));
}

MemoryObjects PointToSummary::reachableMemoryObjects(PointToGraph &ptGraph,
                                                     Pointer *pointer) {
  MemoryObjects reachable;
  unordered_set<Pointer *> worklist;
  worklist.insert(pointer);
  while (!worklist.empty()) {
    auto ptr = *worklist.begin();
    worklist.erase(ptr);
    for (auto pte : getPointees(ptGraph, ptr)) {
      if (reachable.count(pte) == 0) {
        reachable.insert(pte);
        worklist.insert(pte);
      }
    }
  }
  return reachable;
};

MemoryObject *PointToSummary::mustPointToMemory(PointToGraph &ptGraph,
                                                Pointer *pointer) {
  auto ptes = getPointees(ptGraph, pointer);
  if (ptes.size() == 0 || ptes.size() > 1) {
    return nullptr;
  }
  auto memoryObject = dyn_cast<MemoryObject>(*ptes.begin());
  for (auto &[_, memObjPrev] : prevLoopAllocated) {
    if (memoryObject == memObjPrev) {
      return nullptr;
    }
  }
  return memoryObject;
}

Variable *PointToSummary::getVariable(Value *source) {
  auto strippedValue = isa<Instruction>(source) ? strip(source) : source;
  if (variables.count(strippedValue) == 0) {
    variables[strippedValue] = new Variable(strippedValue);
  }
  return variables[strippedValue];
}

MemoryObject *PointToSummary::getMemoryObject(Value *source) {
  auto strippedValue = isa<Instruction>(source) ? strip(source) : source;
  if (memoryObjects.count(strippedValue) == 0) {
    memoryObjects[strippedValue] = new MemoryObject(strippedValue);
  }
  return memoryObjects[strippedValue];
}

MayPointToAnalysis::MayPointToAnalysis(Function *F) {
  this->funcSum = new FunctionSummary(F);
  this->ptSum = nullptr;
}

MayPointToAnalysis::~MayPointToAnalysis() {
  if (funcSum != nullptr) {
    delete funcSum;
  }

  if (ptSum != nullptr) {
    ptSum->eraseDummyObjects();
    delete ptSum;
  }
}

PointToGraph MayPointToAnalysis::mergeAllPredOut(BasicBlock *bb,
                                                 PointToSummary *ptSum) {
  PointToGraph bbIN = PointToGraph();
  for (auto predBB : predecessors(bb)) {
    for (auto &[ptr, ptes] : ptSum->bbOUT.at(predBB)) {
      bbIN[ptr] = unite(ptSum->getPointees(bbIN, ptr), ptes);
    }
  }
  return bbIN;
}

PointToGraph MayPointToAnalysis::FS(Instruction *inst, PointToSummary *ptSum) {
  PointToGraph IN = ptSum->instIN.at(inst);
  PointToGraph GEN, KILL;

  /*
   * For one pointer, get all getPointees of its getPointees.
   * For example, if we have A -> {B, C}, B -> {D, E}, C -> {E, F}.
   * Then ptesOfMyPte(A) = {D, E, F}
   */
  auto ptesOfMyPte = [&](Pointer *ptr) -> MemoryObjects {
    MemoryObjects result;
    for (auto pte : ptSum->getPointees(IN, ptr)) {
      result = unite(result, ptSum->getPointees(IN, pte));
    }
    return result;
  };

  /*
   * Update GEN and KILL,
   * `KILL[ptr] = {}` does not mean KILL = {},
   * it actually means `forall x, ptr -> x belongs to KILL`.
`  */
  auto setptGraph = [&](Pointer *ptr, MemoryObjects newPtes) {
    KILL[ptr] = {};
    if (newPtes.size() > 0) {
      GEN[ptr] = newPtes;
    }
  };

  /*
   * Check if a "prev" memory object is read or written in this loop.
   * See the comments of `ambiguous` in MayPointToAnalysis.hpp.
   */
  auto checkAmbiguity = [&](MemoryObjects ptes) {
    for (auto &[memObj, memObjPrev] : ptSum->prevLoopAllocated) {
      if (ptes.count(memObjPrev) > 0) {
        ptSum->ambiguous.insert(memObj);
        return;
      }
    }
  };

  /*
   * For M1, the memory object allocated by `%1 = tail call i8* @malloc(i64 8)`
   * in a loop, Add M1_prev if there is no record in prevLoopAllocated. For more
   * details, see the comments of `prevLoopAllocated` and `ambiguous` in
   * MayPointToAnalysis.hpp.
   */
  auto tryToAllocatePrev = [&](MemoryObject *heapMemObj) {
    // if heapMemObjPrev already allocated, no need to allocate again
    if (ptSum->prevLoopAllocated.count(heapMemObj) > 0) {
      return;
    }
    // allocate heapMemObjPrev if there is a prev loop
    for (auto &[_, ptes] : IN) {
      if (ptes.count(heapMemObj) > 0) {
        ptSum->prevLoopAllocated[heapMemObj] =
            new MemoryObject(heapMemObj->getSource(), true);
      }
    }
  };

  /*
   * For different types of instructions, calculate GEN and KILL of the point-to
   * info. P.S. in the following case, `M1` refers the memory object allocated
   * by `%1 = ...`. `x` refers to any memory object. %1 = alloca i32 GEN[i] = {
   * (%1, M1) }, KILL[i] = { (%1, x) }
   */
  if (isa<AllocaInst>(inst)) {
    /*
     * %1 = alloca i32
     * GEN[i] = { (%1, M1) }, KILL[i] = { (%1, x) }
     */
    auto allocaInst = dyn_cast<AllocaInst>(inst);
    auto allocaVar = ptSum->getVariable(allocaInst);
    auto allocaMemObj = ptSum->getMemoryObject(allocaInst);
    setptGraph(allocaVar, { allocaMemObj });
  } else if (isa<StoreInst>(inst)) {
    /*
     * store i32* %val, i32** %ptr  (*ptr = val)
     * GEN[i] = { (r, t) | (%ptr, r) ∈ IN[i] and (%val, t) ∈ IN[i] },
     * KILL[i] = { }
     */
    auto storeInst = dyn_cast<StoreInst>(inst);
    auto p = ptSum->getVariable(storeInst->getValueOperand());
    auto q = ptSum->getVariable(storeInst->getPointerOperand());
    checkAmbiguity(ptSum->getPointees(IN, q));
    for (auto r : ptSum->getPointees(IN, q)) {
      setptGraph(r,
                 unite(ptSum->getPointees(IN, r), ptSum->getPointees(IN, p)));
    }

  } else if (isa<LoadInst>(inst)) {
    /*
     * %3 = load i32** %ptr  (%3 = *val)
     * GEN[i] = { (%3, t) | (%ptr, r) ∈ IN[i] and (r, t) ∈ IN[i] },
     * KILL[i] = { (%3, x) }
     */
    auto loadInst = dyn_cast<LoadInst>(inst);
    auto p = ptSum->getVariable(loadInst);
    auto q = ptSum->getVariable(loadInst->getPointerOperand());
    checkAmbiguity(ptSum->getPointees(IN, q));
    setptGraph(p, ptesOfMyPte(q));
  } else if (isa<PHINode>(inst)) {
    /*
     * %5 = phi i64 [ %val1, BB1 ], [ %var2, BB2 ]
     * GEN[i] = { (%5, m) | (%var1, m) ∈ OUT[BB1] || (%var2, m) ∈ OUT[BB2] }
     * KILL[i] = { (%5, x) }
     */
    auto phi = dyn_cast<PHINode>(inst);
    auto phiVar = ptSum->getVariable(phi);
    MemoryObjects phiPtes;
    for (unsigned i = 0; i < phi->getNumIncomingValues(); i++) {
      auto incomingPtr = phi->getIncomingValue(i);
      auto incomingBB = phi->getIncomingBlock(i);
      auto ptesI = ptSum->getPointees(ptSum->bbOUT.at(incomingBB), incomingPtr);
      phiPtes = unite(phiPtes, ptesI);
    }
    setptGraph(phiVar, phiPtes);
  } else if (isa<SelectInst>(inst)) {
    /*
     * %6 = select i1 %cmp, %val1, %var2
     * GEN[i] = { (%6, m) | (%var1, m) ∈ IN[i] || (%var2, m) ∈ IN[i] }
     * KILL[i] = { (%6, x) }
     */
    auto select = dyn_cast<SelectInst>(inst);
    auto selectVar = ptSum->getVariable(select);
    auto truePtr = ptSum->getVariable(select->getTrueValue());
    auto falsePtr = ptSum->getVariable(select->getFalseValue());
    auto truePtes = ptSum->getPointees(IN, truePtr);
    auto falsePtes = ptSum->getPointees(IN, falsePtr);
    setptGraph(selectVar, unite(truePtes, falsePtes));
  } else if (isa<GetElementPtrInst>(inst)) {
    /*
     * %4 = getelementptr inbounds i8*, i8** %ptr, i64 1
     * GEN[i] = { (%4, m) | (%ptr, m) ∈ IN[i] }
     * KILL[i] = { (%4, x) }
     */
    auto gep = dyn_cast<GetElementPtrInst>(inst);
    auto gepVar = ptSum->getVariable(gep);
    auto ptr = ptSum->getVariable(gep->getOperand(0));
    setptGraph(gepVar, ptSum->getPointees(IN, ptr));
  } else if (isa<CallInst>(inst)) {
    auto callInst = dyn_cast<CallInst>(inst);
    auto fname = getCalledFuncName(callInst);
    if (funcSum->mallocInsts.count(callInst) > 0
        || funcSum->callocInsts.count(callInst) > 0) {
      /*
       * %2 = call noalias i8* @malloc(i64 16)
       * GEN[i] = { (%2, M2) } ∪ { (m, M2_prev) | (m, M2) ∈ IN[i] & m != %2 }
       * KILL[i] = { (%2, x) } ∪ { (x, M2) }
       */
      auto heapVar = ptSum->getVariable(callInst);
      auto heapMemObj = ptSum->getMemoryObject(callInst);
      tryToAllocatePrev(heapMemObj);
      if (ptSum->prevLoopAllocated.count(heapMemObj) > 0) {
        auto memObjPrev = ptSum->prevLoopAllocated.at(heapMemObj);
        for (auto &[ptr, ptes] : IN) {
          if (ptes.count(heapMemObj) > 0) {
            setptGraph(ptr, replace(ptes, heapMemObj, memObjPrev));
          }
        }
      }
      setptGraph(heapVar, { heapMemObj });
    } else if (funcSum->reallocInsts.count(callInst) > 0) {
      /*
       * %9 = call void realloc(i8* %7, i64 %8)
       * GEN[i] = { (%9, m) | (%7, m) ∈ IN[i] }
       * GEN[i] = { (%9, x) }
       */
      auto reallocVar = ptSum->getVariable(callInst);
      auto ptr = ptSum->getVariable(callInst->getArgOperand(0));
      setptGraph(reallocVar, ptSum->getPointees(IN, ptr));
    } else if (funcSum->freeInsts.count(callInst) > 0) {
      /*
       * call void @free(i8* %7)
       * GEN[i] = { }
       * KILL[i] = { (x, m) | (%7, m) ∈ IN[i] & ( (%7, n) ∉ IN[i] ∀ n != m) }
       */
      auto ptGraph = ptSum->instIN.at(callInst);
      auto ptr = ptSum->getVariable(callInst->getArgOperand(0));
      if (auto memObj = ptSum->mustPointToMemory(ptGraph, ptr)) {
        for (auto &[ptr, ptes] : IN) {
          if (ptes.count(memObj) > 0) {
            setptGraph(ptr, minus(ptes, { memObj }));
          }
        }
      }
    } else if (callInst->isLifetimeStartOrEnd()) {
      // do nothing
    } else if (READ_ONLY_LIB_FUNCTIONS.count(getCalledFuncName(callInst)) > 0) {
      // do nothing
    } else if (isa<MemCpyInst>(callInst)) {
      /*
       * call void @memcpy(i8* %1, i8* %2, i64 16)
       * GEN[i] = { (m, t) | (%1, m) ∈ IN[i] & (%2, t) ∈ IN[i] }
       * KILL[i] = { }
       */
      auto memcpyInst = dyn_cast<MemCpyInst>(callInst);
      auto dest = ptSum->getVariable(memcpyInst->getRawDest());
      auto source = ptSum->getVariable(memcpyInst->getRawSource());
      for (auto destMemObj : ptSum->getPointees(IN, dest)) {
        auto oldPtes = ptSum->getPointees(IN, destMemObj);
        auto newPtes = ptesOfMyPte(source);
        setptGraph(destMemObj, unite(oldPtes, newPtes));
        ptSum->mustHeap.insert(destMemObj);
      }
    } else {
      /*
       * GLOABL_AND_NLMO = { NLMO } ∪ { GLOBAL_MEMORY_OBJECTS }
       * ESCAPED = { reachable(%arg, IN[i]) | %arg ∈ callInst->arg_operands() }}
       * OUTSIDE = ESCAPED ∪ GLOABL_AND_NLMO
       * GEN[i] = { (%8, n) | n ∈ OUTSIDE } ∪ { (a, b) | ∀ a, b ∈ OUTSIDE }
       * KILL[i] = { (%8, x) }
       * */
      auto escapedMemObjs = ptSum->escaped;
      for (auto &arg : callInst->arg_operands()) {
        auto escapedVariable = ptSum->getVariable(
            strip(callInst->getArgOperand(arg.getOperandNo())));
        escapedMemObjs =
            unite(escapedMemObjs,
                  ptSum->reachableMemoryObjects(IN, escapedVariable));
      }

      auto GlobalAndNonLocalMemObjs =
          unite({ ptSum->nonLocalMemoryObject }, ptSum->globalMemoryObjects);
      auto outSideMemObjs = unite(escapedMemObjs, GlobalAndNonLocalMemObjs);
      auto retVar = ptSum->getVariable(callInst);

      setptGraph(retVar, outSideMemObjs);
      for (auto memObj : outSideMemObjs) {
        setptGraph(memObj, outSideMemObjs);
      }
    }
  }

  PointToGraph OUT = add(GEN, minus(IN, KILL));
  // if some memObj is pointed by "nonLocalMemoryObject", it is escaped
  ptSum->escaped =
      ptSum->reachableMemoryObjects(OUT, ptSum->nonLocalMemoryObject);
  ptSum->escaped = minus(ptSum->escaped, ptSum->globalMemoryObjects);
  ptSum->escaped.erase(ptSum->nonLocalMemoryObject);

  return OUT;
};

PointToSummary *MayPointToAnalysis::getPointToSummary() {
  errs() << "Enter PointToSummary for function: "
         << funcSum->F->getFunction().getName() << "\n";

  if (ptSum) {
    return ptSum;
  }

  ptSum = new PointToSummary(funcSum);

  auto entryBB = &funcSum->F->getEntryBlock();

  unordered_set<BasicBlock *> workList({ entryBB });
  unordered_set<BasicBlock *> visited;

  /*
   * For basic block bb, compute the point-to info for IN[bb].
   */
  auto bbIn = [&](BasicBlock *bb) -> PointToGraph {
    if (bb == entryBB) {
      PointToGraph entryIN;

      for (auto &G : ptSum->M->globals()) {
        auto globalVar = ptSum->getVariable(&G);
        auto globalMemObj = ptSum->getMemoryObject(&G);
        entryIN[globalVar] = { globalMemObj };
      }

      if (!funcSum->F || funcSum->F->getName() != "main") {
        /*
         * For the entry basic block of function that is not main,
         * IN[bb] will collect information of global variables and arguments.
         * In this case, or memory objects will conservatively point to each
         * other.
         */
        auto GlobalAndNonLocalMemObjs =
            unite({ ptSum->nonLocalMemoryObject }, ptSum->globalMemoryObjects);
        for (auto &arg : funcSum->F->args()) {
          auto argVar = ptSum->getVariable(&arg);
          entryIN[argVar] = GlobalAndNonLocalMemObjs;
        }
        for (auto memObj : GlobalAndNonLocalMemObjs) {
          entryIN[memObj] = GlobalAndNonLocalMemObjs;
        }
      } else {
        /*
         * For the entry basic block of main function, global memory objects are
         * not modified. So we can assume that global memory objects will not
         * point to other global memory objects.
         *
         * The only exception is the string array.
         * if we have three global variable array = ["str1", "str2", "str3"] in
         * c, it will be translated to llvm ir as:
         *
         * @.str1 = private unnamed_addr constant [5 x i8] c"str1\00", align 1
         * @.str2 = private unnamed_addr constant [5 x i8] c"str2\00", align 1
         * @array = private unnamed_addr constant [2 x i8*] [
         *  i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.1, i32 0, i32
         * 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.2, i32 0,
         * i32 0)
         * ]
         *
         * Here the point-to graph is :
         *
         * Variable(@.str1) -> MemoryObject(@.str1);
         * Variable(@.str2) -> MemoryObject(@.str2);
         * Variable(@array) -> MemoryObject(@array);
         * MemoryObject(@array) -> MemoryObject(@.str1);
         * MemoryObject(@array) -> MemoryObject(@.str2);
         */
        for (auto globalMemObj : ptSum->globalMemoryObjects) {
          auto globalVar = dyn_cast<GlobalVariable>(globalMemObj->getSource());
          assert(globalVar != nullptr);
          auto globalVarType = globalVar->getValueType();

          if (globalVarType->isArrayTy()) {
            if (auto constantArray =
                    dyn_cast<ConstantArray>(globalVar->getInitializer())) {
              for (auto &element : constantArray->operands()) {
                if (auto gep = dyn_cast<GEPOperator>(element)) {
                  auto gepVar = gep->getPointerOperand();
                  auto gepVarMemObj = ptSum->getMemoryObject(gepVar);
                  entryIN[globalMemObj] = { gepVarMemObj };
                }
              }
            }
          }
        }
      }
      return entryIN;
    } else {
      /*
       * For a normal basic block, IN[bb] is the union of OUT[pred] for all
       * predessors.
       */
      return mergeAllPredOut(bb, ptSum);
    }
  };

  auto updateBBOut = [&](BasicBlock *bb) -> bool {
    auto terminator = bb->getTerminator();
    auto ptModified = ptSum->bbOUT.at(bb) != ptSum->instOUT.at(terminator);
    ptSum->bbOUT[bb] = ptSum->instOUT.at(terminator);
    return ptModified;
  };

  while (!workList.empty()) {
    BasicBlock *bb = *workList.begin();
    workList.erase(workList.begin());
    visited.insert(bb);

    ptSum->bbIN[bb] = bbIn(bb);

    Instruction *prev = nullptr;
    for (auto &inst : *bb) {

      ptSum->instIN[&inst] =
          (prev == nullptr) ? ptSum->bbIN.at(bb) : ptSum->instOUT.at(prev);
      ptSum->instOUT[&inst] = FS(&inst, ptSum);
      prev = &inst;
    }

    auto bbOutModified = updateBBOut(bb);
    for (auto succ : successors(bb)) {
      if (bbOutModified || visited.count(succ) == 0) {
        workList.insert(succ);
      }
    }
  }

  errs() << "Exit PointToSummary for function"
         << funcSum->F->getFunction().getName() << "\n";
  return ptSum;
}

LiveMemorySummary *MayPointToAnalysis::getLiveMemorySummary() {

  auto ptSum = getPointToSummary();

  MemoryObjects allocable = [&]() -> MemoryObjects {
    MemoryObjects fixedSized;
    auto heapAllocInsts = funcSum->mallocInsts;
    heapAllocInsts.insert(funcSum->callocInsts.begin(),
                          funcSum->callocInsts.end());

    for (auto heapAllocInst : heapAllocInsts) {
      auto heapMemObj = ptSum->getMemoryObject(heapAllocInst);
      if (getCalledFuncName(heapAllocInst) == MALLOC) {
        if (dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))) {
          fixedSized.insert(heapMemObj);
        }
      } else if (getCalledFuncName(heapAllocInst) == CALLOC) {
        if (dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))
            && dyn_cast<ConstantInt>(heapAllocInst->getOperand(1))) {
          fixedSized.insert(heapMemObj);
        }
      }
    }
    return fixedSized;
  }();

  allocable = minus(allocable, ptSum->mustHeap);
  allocable = minus(allocable, ptSum->escaped);
  allocable = minus(allocable, ptSum->ambiguous);

  auto [removable, unknownWhetherFreed] =
      [&]() -> pair<unordered_set<CallInst *>, MemoryObjects> {
    MemoryObjects unknownWhetherFreed;
    for (auto freeInst : funcSum->freeInsts) {
      auto ptr = freeInst->getArgOperand(0);
      auto mayBeFreed = ptSum->getPointees(ptSum->instIN.at(freeInst), ptr);
      for (auto memObj : mayBeFreed) {
        if (allocable.count(memObj) == 0) {
          unknownWhetherFreed = unite(unknownWhetherFreed, mayBeFreed);
        }
      }
    }

    unordered_set<CallInst *> removable = funcSum->freeInsts;
    for (auto freeInst : funcSum->freeInsts) {
      auto ptr = freeInst->getArgOperand(0);
      auto mayBeFreed = ptSum->getPointees(ptSum->instIN.at(freeInst), ptr);
      for (auto memObj : mayBeFreed) {
        if (unknownWhetherFreed.count(memObj) > 0) {
          removable.erase(freeInst);
        }
      }
    }

    return make_pair(removable, unknownWhetherFreed);
  }();

  allocable = minus(allocable, unknownWhetherFreed);

  // Remove free instructions of optimized memory objects
  // Also check if the memory object is fixed sized
  LiveMemorySummary *memSum = new LiveMemorySummary();
  memSum->removable = removable;
  memSum->allocable = [&]() {
    unordered_set<CallInst *> result;
    for (auto memObj : allocable) {
      auto heapAllocInst = dyn_cast<CallInst>(memObj->getSource());
      assert(heapAllocInst);
      result.insert(heapAllocInst);
    }
    return result;
  }();

  return memSum;
}

FunctionSummary *MayPointToAnalysis::getFunctionSummary() {
  return this->funcSum;
}

bool MayPointToAnalysis::canBeClonedToStack(GlobalVariable *globalVar,
                                            LoopForest *loopForest) {
  /*
   * We only clone global variables to stakc for main function.
   */
  auto currentF = funcSum->F;
  auto mainF = funcSum->M->getFunction("main");
  if (currentF != mainF) {
    errs() << "Function " << currentF->getName()
           << " is not main function, do not clone global variable "
           << globalVar->getName() << " to stack.\n";
    return false;
  }

  /*
   * Global variable shall not be accessed in another function.
   * If accessed, do not clone it.
   *
   * If some function @F() is called in current function,
   * we will conservatively assume the global
   * memory object is modified and choose not to clone it.
   */
  getPointToSummary();
  auto globalMemObj = ptSum->getMemoryObject(globalVar);
  if (funcSum->unknownFuntctionCalls.size() > 0) {
    return false;
  }

  /*
   * If global variable is never written in a loop,
   * which means the global variable is read-only or never used in a loop,
   * cloning contributes nothing to parallelization, do not clone it.
   */
  unordered_set<LoopStructure *> loopsUsingGlobalVar;
  for (auto tree : loopForest->getTrees()) {
    auto loop = tree->getLoop();
    auto loopFunction = loop->getFunction();
    if (loopFunction != mainF) {
      continue;
    }
    for (auto inst : loop->getInstructions()) {
      if (auto storeInst = dyn_cast<StoreInst>(inst)) {
        auto IN = ptSum->instIN.at(storeInst);
        auto ptr = strip(storeInst->getPointerOperand());
        auto mayBeStored = ptSum->getPointees(IN, ptr);
        auto globalMemObj = ptSum->getMemoryObject(globalVar);
        if (mayBeStored.count(globalMemObj) > 0) {
          loopsUsingGlobalVar.insert(loop);
        }
      }
    }
  }
  if (loopsUsingGlobalVar.empty()) {
    errs() << "Global variable " << globalVar->getName()
           << " is never written in a loop, do not clone it.\n";
    return false;
  }

  errs() << "Global variable " << globalVar->getName()
         << " can be cloned to stack.\n";
  return true;
}

}; // namespace llvm::noelle
