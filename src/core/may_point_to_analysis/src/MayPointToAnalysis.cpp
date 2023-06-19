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

#include "noelle/core/MayPointToAnalysis.hpp"
#include "MayPointToAnalysisUtils.hpp"
#include <unordered_set>

using namespace std;

namespace llvm::noelle {

MayPointToAnalysis::MayPointToAnalysis(Function *F) {
  this->functionSummary = new FunctionSummary(F);
  this->pointToSummary = nullptr;
}

MayPointToAnalysis::~MayPointToAnalysis() {
  if (functionSummary != nullptr) {
    delete functionSummary;
  }

  if (pointToSummary != nullptr) {
    delete pointToSummary;
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
  auto funcSum = this->functionSummary;
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
  } else if (isa<CallBase>(inst)) {
    auto callInst = dyn_cast<CallBase>(inst);
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
    } else if (isLifetimeIntrinsic(callInst)) {
      // do nothing
    } else if (READ_ONLY_LIB_FUNCTIONS_WITH_SUFFIX.count(
                   getCalledFuncName(callInst))
               > 0) {
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

PointToSummary *MayPointToAnalysis::getPointToSummary(void) {
  errs() << "Enter PointToSummary for function: "
         << functionSummary->F->getFunction().getName() << "\n";

  if (pointToSummary) {
    return pointToSummary;
  }

  auto funcSum = functionSummary;
  auto ptSum = new PointToSummary(funcSum);

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

          if (!(globalVarType->isArrayTy() && globalVar->hasInitializer())) {
            continue;
          }

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

  pointToSummary = ptSum;
  return ptSum;
}

/*
 * Compute @malloc() or @calloc() insts that could be transformed to allocaInst.
 * Compute @free() insts that could be removed becase if @malloc() is
 * transformed to allocaInst, the corresponding @free() must be removed.
 */
LiveMemorySummary *MayPointToAnalysis::getLiveMemorySummary(void) {

  auto funcSum = functionSummary;
  auto ptSum = getPointToSummary();

  /*
   * Only fixed size @malloc(), such as %1 = tail call i8* @malloc(i64 8), can
   * be transformed to allocaInst. Otherwise, it may cause stack overflow.
   */
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

  auto [removable, notAllocable] =
      [&]() -> pair<unordered_set<CallBase *>, MemoryObjects> {
    /*
     * We have:
     * %1 = call i8* @malloc(i64 8), %1 -> M1 (M1 is the memory object allocated
     * by @malloc) and %2 = call i8* @malloc(i64 8), %2 -> M2 (M2 is the memory
     * object allocated by @malloc)
     *
     * Assume M1 is can be transformed to allocaInst, M2 escapes and therefore
     * cannot be transformed to allocaInst, and we have a free instruction
     * %7 = call i8* @free(i8* %6), where %6 may point to M1 or M2.
     *
     * In this case:
     * M2 is not allocable because it escapes, it remains a @malloc().
     * To maintain the original semantics, we cannot remove %7, because it
     * may free M2. Since %7 may also free M1, we cannot transform %1 to
     * allocaInst since that may cause segfault.
     *
     * Therefore, for any free inst: %7 = call i8* @free(i8* %6)
     * if any memory object pointed by %6 is not allocable,
     * then all memory objects pointed by %6 are not allocable.
     */
    MemoryObjects notAllocable;
    for (auto freeInst : funcSum->freeInsts) {
      auto ptr = freeInst->getArgOperand(0);
      auto mayBeFreed = ptSum->getPointees(ptSum->instIN.at(freeInst), ptr);
      for (auto memObj : mayBeFreed) {
        if (allocable.count(memObj) == 0) {
          notAllocable = unite(notAllocable, mayBeFreed);
        }
      }
    }

    unordered_set<CallBase *> removable = funcSum->freeInsts;
    for (auto freeInst : funcSum->freeInsts) {
      auto ptr = freeInst->getArgOperand(0);
      auto mayBeFreed = ptSum->getPointees(ptSum->instIN.at(freeInst), ptr);
      for (auto memObj : mayBeFreed) {
        if (notAllocable.count(memObj) > 0) {
          removable.erase(freeInst);
        }
      }
    }

    return make_pair(removable, notAllocable);
  }();

  allocable = minus(allocable, notAllocable);

  LiveMemorySummary *memSum = new LiveMemorySummary();
  memSum->removable = removable;
  memSum->allocable = [&](void) {
    unordered_set<CallBase *> result;
    for (auto memObj : allocable) {
      auto heapAllocInst = dyn_cast<CallBase>(memObj->getSource());
      assert(heapAllocInst);
      result.insert(heapAllocInst);
    }
    return result;
  }();

  return memSum;
}

FunctionSummary *MayPointToAnalysis::getFunctionSummary(void) {
  return this->functionSummary;
}

}; // namespace llvm::noelle
