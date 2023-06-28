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
                            unordered_set<Function *> &visited) {

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
    modified |= funcPtGraph->addPointees(allocaVar, { allocaMemObj });
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
      modified |= funcPtGraph->addPointees(r, funcPtGraph->getPointees(p));
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
    modified |= funcPtGraph->addPointees(p, ptesOfMyPte(q));
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
    modified |= funcPtGraph->addPointees(phiVar, phiPtes);
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
    modified |= funcPtGraph->addPointees(selectVar, unite(truePtes, falsePtes));
  } else if (isa<GetElementPtrInst>(inst)) {
    /*
     * %4 = getelementptr inbounds i8*, i8** %ptr, i64 1
     * GEN[i] = { (%4, m) | (%ptr, m) ∈ IN[i] }
     * KILL[i] = { (%4, x) }
     */
    auto gep = dyn_cast<GetElementPtrInst>(inst);
    auto gepVar = ptSum->getVariable(gep);
    auto ptr = ptSum->getVariable(gep->getOperand(0));
    modified |= funcPtGraph->addPointees(gepVar, funcPtGraph->getPointees(ptr));
  } else if (isa<CallBase>(inst)) {
    auto callInst = dyn_cast<CallBase>(inst);
    auto funcType = getMPAFunctionType(callInst);

    if (funcType == MALLOC || funcType == CALLOC) {
      /*
       * %2 = call noalias i8* @malloc(i64 16)
       * GEN[i] = { (%2, M2) } ∪ { (m, M2_prev) | (m, M2) ∈ IN[i] & m != %2 }
       * KILL[i] = { (%2, x) } ∪ { (x, M2) }
       */
      auto heapVar = ptSum->getVariable(callInst);
      auto heapMemObj = ptSum->getMemoryObject(callInst);
      modified |= funcPtGraph->addPointees(heapVar, { heapMemObj });
    } else if (funcType == REALLOC) {
      /*
       * %9 = call void realloc(i8* %7, i64 %8)
       * GEN[i] = { (%9, m) | (%7, m) ∈ IN[i] }
       * GEN[i] = { (%9, x) }
       */
      auto reallocVar = ptSum->getVariable(callInst);
      auto ptr = ptSum->getVariable(callInst->getArgOperand(0));
      modified |=
          funcPtGraph->addPointees(reallocVar, funcPtGraph->getPointees(ptr));
    } else if (funcType == FREE) {
      /*
       * call void @free(i8* %7)
       * GEN[i] = { }
       * KILL[i] = { (x, m) | (%7, m) ∈ IN[i] & ( (%7, n) ∉ IN[i] ∀ n != m) }
       */
    } else if (funcType == INTRINSIC) {
      // do nothing
    } else if (funcType == READ_ONLY) {
      // do nothing
    } else if (funcType == MEM_COPY) {
      /*
       * call void @memcpy(i8* %1, i8* %2, i64 16)
       * GEN[i] = { (m, t) | (%1, m) ∈ IN[i] & (%2, t) ∈ IN[i] }
       * KILL[i] = { }
       */
      auto memcpyInst = dyn_cast<MemCpyInst>(callInst);
      auto dest = ptSum->getVariable(memcpyInst->getRawDest());
      auto source = ptSum->getVariable(memcpyInst->getRawSource());
      for (auto destMemObj : funcPtGraph->getPointees(dest)) {
        auto newPtes = ptesOfMyPte(source);
        modified |= funcPtGraph->addPointees(destMemObj, newPtes);
        funcSum->mustHeap.insert(destMemObj);
      }
    } else if (funcType == USER_DEFINED) {
      auto calleeFunc = callInst->getCalledFunction();
      auto calleeSum = ptSum->getFunctionSummary(calleeFunc);
      auto calleePtGraph = calleeSum->functionPointToGraph;

      for (auto &arg : callInst->arg_operands()) {
        auto operand = ptSum->getVariable(
            strip(callInst->getArgOperand(arg.getOperandNo())));
        auto argument =
            ptSum->getVariable(calleeFunc->args().begin() + arg.getOperandNo());
        calleePtGraph->addPointees(argument, funcPtGraph->getPointees(operand));

        for (auto memObj : funcPtGraph->getReachableMemoryObjects(operand)) {
          calleePtGraph->addPointees(memObj, funcPtGraph->getPointees(memObj));
        }
      }
      for (auto &G : ptSum->M.globals()) {
        auto globalVar = ptSum->getVariable(&G);
        calleePtGraph->addPointees(globalVar,
                                   funcPtGraph->getPointees(globalVar));

        for (auto memObj : funcPtGraph->getReachableMemoryObjects(globalVar)) {
          calleePtGraph->addPointees(memObj, funcPtGraph->getPointees(memObj));
        }
      }

      updateFunctionSummaryUntilFixedPoint(calleeFunc, visited);

      auto returnVariable = ptSum->getVariable(callInst);
      auto returnValue = calleeSum->returnValue;

      modified |= funcPtGraph->addPointees(returnVariable, returnValue);

      for (auto &G : ptSum->M.globals()) {
        auto globalVar = ptSum->getVariable(&G);
        for (auto memObj :
             calleePtGraph->getReachableMemoryObjects(globalVar)) {
          modified |=
              funcPtGraph->addPointees(memObj,
                                       calleePtGraph->getPointees(memObj));
        }
      }

    } else if (funcType == UNKNOWN) {
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
                  funcPtGraph->getReachableMemoryObjects(escapedVariable));
      }

      auto retVar = ptSum->getVariable(callInst);
      auto outsideMemObjs =
          unite(escapedMemObjs, { ptSum->unknownMemoryObject });
      modified |= funcPtGraph->addPointees(retVar, outsideMemObjs);
      for (auto memObj : outsideMemObjs) {
        modified |= funcPtGraph->addPointees(memObj, outsideMemObjs);
      }
    }
  }

  return modified;
};

void MayPointToAnalysis::updateFunctionSummaryUntilFixedPoint(
    Function *currentF,
    unordered_set<Function *> &visited) {
  assert(ptSum != nullptr);

  if (visited.count(currentF) > 0) {
    return;
  }

  visited.insert(currentF);
  auto funcSum = ptSum->getFunctionSummary(currentF);

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
  auto mainFuncSum = ptSum->getFunctionSummary(mainF);
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

  unordered_set<Function *> visited;
  updateFunctionSummaryUntilFixedPoint(mainF, visited);
  return ptSum;
}

}; // namespace llvm::noelle
