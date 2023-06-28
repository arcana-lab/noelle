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

using namespace std;

namespace llvm::noelle {

MayPointToAnalysis::MayPointToAnalysis() {
  this->ptSum = nullptr;
}

MayPointToAnalysis::~MayPointToAnalysis() {
  if (ptSum != nullptr) {
    delete ptSum;
  }
}

bool MayPointToAnalysis::FS(FunctionSummary *funcSum,
                            Instruction *inst,
                            set<FunctionCall> &visited) {

  auto funcPtGraph = funcSum->functionPointToGraph;
  auto modified = false;
  /*
   * For one pointer, get all getPointees of its getPointees.
   * For example, if we have A -> {B, C}, B -> {D, E}, C -> {E, F}.
   * Then ptesOfMyPte(A) = {D, E, F}
   */
  auto ptesOfMyPte = [&](Pointer *ptr) -> MemoryObjects {
    MemoryObjects result;
    for (auto pte : funcPtGraph->getPointees(ptr)) {
      result = unite(result, funcPtGraph->getPointees(pte));
    }
    return result;
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
    modified |= funcPtGraph->setPointees(allocaVar, { allocaMemObj });
  } else if (isa<StoreInst>(inst)) {
    /*
     * *q = p
     * store i32* %val, i32** %ptr  (*ptr = val)
     * GEN[i] = { (r, t) | (%ptr, r) ∈ IN[i] and (%val, t) ∈ IN[i] },
     * KILL[i] = { }
     */
    auto storeInst = dyn_cast<StoreInst>(inst);
    auto p = ptSum->getVariable(storeInst->getValueOperand());
    auto q = ptSum->getVariable(storeInst->getPointerOperand());
    // checkAmbiguity(ptSum->getPointees(IN, q));
    for (auto r : funcPtGraph->getPointees(q)) {
      modified |= funcPtGraph->setPointees(
          r,
          unite(funcPtGraph->getPointees(r), funcPtGraph->getPointees(p)));
    }

  } else if (isa<LoadInst>(inst)) {
    /*
     * p = *q
     * %3 = load i32** %ptr  (%3 = *val)
     * GEN[i] = { (%3, t) | (%ptr, r) ∈ IN[i] and (r, t) ∈ IN[i] },
     * KILL[i] = { (%3, x) }
     */
    auto loadInst = dyn_cast<LoadInst>(inst);
    auto p = ptSum->getVariable(loadInst);
    auto q = ptSum->getVariable(loadInst->getPointerOperand());
    modified |= funcPtGraph->setPointees(p, ptesOfMyPte(q));
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
      auto incomingPtr = ptSum->getVariable(phi->getIncomingValue(i));
      auto ptesI = funcPtGraph->getPointees(incomingPtr);
      phiPtes = unite(phiPtes, ptesI);
    }
    modified |= funcPtGraph->setPointees(phiVar, phiPtes);
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
    auto truePtes = funcPtGraph->getPointees(truePtr);
    auto falsePtes = funcPtGraph->getPointees(falsePtr);
    modified |= funcPtGraph->setPointees(selectVar, unite(truePtes, falsePtes));
  } else if (isa<GetElementPtrInst>(inst)) {
    /*
     * %4 = getelementptr inbounds i8*, i8** %ptr, i64 1
     * GEN[i] = { (%4, m) | (%ptr, m) ∈ IN[i] }
     * KILL[i] = { (%4, x) }
     */
    auto gep = dyn_cast<GetElementPtrInst>(inst);
    auto gepVar = ptSum->getVariable(gep);
    auto ptr = ptSum->getVariable(gep->getOperand(0));
    modified |= funcPtGraph->setPointees(gepVar, funcPtGraph->getPointees(ptr));
  } else if (isa<CallBase>(inst)) {
    auto callInst = dyn_cast<CallBase>(inst);
    auto calledFunc = callInst->getCalledFunction();
    auto functionCall = make_pair(funcSum->currentF, calledFunc);

    if (funcSum->mallocInsts.count(callInst) > 0
        || funcSum->callocInsts.count(callInst) > 0) {
      /*
       * %2 = call noalias i8* @malloc(i64 16)
       * GEN[i] = { (%2, M2) } ∪ { (m, M2_prev) | (m, M2) ∈ IN[i] & m != %2 }
       * KILL[i] = { (%2, x) } ∪ { (x, M2) }
       */
      auto heapVar = ptSum->getVariable(callInst);
      auto heapMemObj = ptSum->getMemoryObject(callInst);
      modified |= funcPtGraph->setPointees(heapVar, { heapMemObj });
    } else if (funcSum->reallocInsts.count(callInst) > 0) {
      /*
       * %9 = call void realloc(i8* %7, i64 %8)
       * GEN[i] = { (%9, m) | (%7, m) ∈ IN[i] }
       * GEN[i] = { (%9, x) }
       */
      auto reallocVar = ptSum->getVariable(callInst);
      auto ptr = ptSum->getVariable(callInst->getArgOperand(0));
      modified |=
          funcPtGraph->setPointees(reallocVar, funcPtGraph->getPointees(ptr));
    } else if (funcSum->freeInsts.count(callInst) > 0) {
      /*
       * call void @free(i8* %7)
       * GEN[i] = { }
       * KILL[i] = { (x, m) | (%7, m) ∈ IN[i] & ( (%7, n) ∉ IN[i] ∀ n != m) }
       */
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
      for (auto destMemObj : funcPtGraph->getPointees(dest)) {
        auto oldPtes = funcPtGraph->getPointees(destMemObj);
        auto newPtes = ptesOfMyPte(source);
        modified |=
            funcPtGraph->setPointees(destMemObj, unite(oldPtes, newPtes));
        funcSum->mustHeap.insert(destMemObj);
      }
    } else if (ptSum->funcSums.count(functionCall) > 0) {
      auto calleeSum = ptSum->funcSums.at(functionCall);
      auto calleePtGraph = calleeSum->functionPointToGraph;
      for (auto &arg : callInst->arg_operands()) {
        auto operand = ptSum->getVariable(
            strip(callInst->getArgOperand(arg.getOperandNo())));
        auto argument =
            ptSum->getVariable(calledFunc->args().begin() + arg.getOperandNo());
        calleePtGraph->setPointees(argument, funcPtGraph->getPointees(operand));
      }
      updateFunctionSummaryUntilFixedPoint(functionCall, visited);

      auto returnVariable = ptSum->getVariable(callInst);
      auto returnValue = calleeSum->returnValue;
      modified |= funcPtGraph->setPointees(returnVariable, returnValue);
    } else {
      /*
       * GLOABL_AND_NLMO = { NLMO } ∪ { GLOBAL_MEMORY_OBJECTS }
       * ESCAPED = { reachable(%arg, IN[i]) | %arg ∈ callInst->arg_operands() }}
       * OUTSIDE = ESCAPED ∪ GLOABL_AND_NLMO
       * GEN[i] = { (%8, n) | n ∈ OUTSIDE } ∪ { (a, b) | ∀ a, b ∈ OUTSIDE }
       * KILL[i] = { (%8, x) }
       * */
      MemoryObjects escapedMemObjs;
      for (auto &arg : callInst->arg_operands()) {
        auto escapedVariable = ptSum->getVariable(
            strip(callInst->getArgOperand(arg.getOperandNo())));
        escapedMemObjs =
            unite(escapedMemObjs,
                  funcPtGraph->reachableMemoryObjects(escapedVariable));
      }

      auto retVar = ptSum->getVariable(callInst);

      modified |= funcPtGraph->setPointees(retVar, escapedMemObjs);
      for (auto memObj : escapedMemObjs) {
        modified |= funcPtGraph->setPointees(memObj, escapedMemObjs);
      }
    }
  }

  return modified;
};

void MayPointToAnalysis::updateFunctionSummaryUntilFixedPoint(
    FunctionCall functionCall,
    set<FunctionCall> &visited) {
  assert(ptSum != nullptr);
  assert(ptSum->funcSums.count(functionCall) > 0);

  if (visited.count(functionCall) > 0) {
    return;
  }

  visited.insert(functionCall);
  auto funcSum = ptSum->funcSums.at(functionCall);
  auto currentF = functionCall.second;

  bool modified = true;
  while (modified) {
    modified = false;
    for (auto &BB : *currentF) {
      for (auto &I : BB) {
        modified |= FS(funcSum, &I, visited);
      }
    }
  }

  return;
}

PointToSummary *MayPointToAnalysis::getPointToSummary(Module &M,
                                                      CallGraph *callGraph) {

  if (ptSum != nullptr) {
    return ptSum;
  }

  ptSum = new PointToSummary(M, callGraph);
  auto mainF = M.getFunction("main");
  auto mainFuncCall = std::make_pair(nullptr, mainF);
  auto mainFuncSum = ptSum->funcSums.at(mainFuncCall);
  auto mainFuncPtGraph = mainFuncSum->functionPointToGraph;

  for (auto &G : M.globals()) {
    auto globalVar = ptSum->getVariable(&G);
    auto globalMemObj = ptSum->getMemoryObject(&G);
    mainFuncPtGraph->setPointees(globalVar, { globalMemObj });
  }

  for (auto globalMemObj : ptSum->globalMemoryObjects) {
    auto llvmGlobalVar = dyn_cast<GlobalVariable>(globalMemObj->getSource());
    assert(llvmGlobalVar != nullptr);
    auto globalVarType = llvmGlobalVar->getValueType();

    if (!(globalVarType->isArrayTy() && llvmGlobalVar->hasInitializer())) {
      continue;
    }

    if (auto constantArray =
            dyn_cast<ConstantArray>(llvmGlobalVar->getInitializer())) {
      for (auto &element : constantArray->operands()) {
        if (auto gep = dyn_cast<GEPOperator>(element)) {
          auto gepVar = gep->getPointerOperand();
          auto gepMemObj = ptSum->getMemoryObject(gepVar);
          mainFuncPtGraph->setPointees(globalMemObj, { gepMemObj });
        }
      }
    }
  }

  set<FunctionCall> visited;
  updateFunctionSummaryUntilFixedPoint(mainFuncCall, visited);
  return ptSum;
}

}; // namespace llvm::noelle
