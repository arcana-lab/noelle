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
#include "PrivatizerManager.hpp"
#include "llvm/IR/Constants.h"
#include <numeric>

namespace llvm::noelle {

bool PrivatizerManager::applyGlobalToStack(Noelle &noelle,
                                           PointToSummary *ptSum) {
  bool modified = false;

  for (auto &G : ptSum->M.globals()) {
    auto privatizableFunctions = getPrivatizableFunctions(&G, ptSum);

    auto fname = funcSum->currentF->getName();
    auto globalVarName = G.getName();

    if (privatizableFunctions.empty()) {
      errs()
          << predix << globalVarName << " can't be privatized to any function.";
      continue;
    }

    for (auto currentF : privatizableFunctions) {
      auto suffix = " in function " + fname + "\n";
      errs() << prefix << "Try to privatize " << globalVarName << suffix;

      auto allocationSize = getAllocationSize(heapAllocInst);
      if (!stackHasEnoughSpaceForNewAllocaInst(allocationSize,
                                               stackMemoryUsage)) {
        errs() << prefix
               << "Stack memory usage exceeds the limit, can't privatize "
               << globalVarName << suffix;
        continue;
      }

      modified = true;
      auto &entryBlock = currentF->getEntryBlock();
      IRBuilder<> entryBuilder(entryBlock.getFirstNonPHI());
      Type *globalVarType = G.getValueType();
      AllocaInst *allocaInst =
          entryBuilder.CreateAlloca(globalVarType, nullptr, "");

      if (G.hasInitializer()) {
        auto initializer = G.getInitializer();
        if (isa<ConstantAggregateZero>(initializer)
            && globalVarType->isArrayTy()) {
          auto typesManager = noelle.getTypesManager();
          auto sizeInByte = typesManager->getSizeOfType(globalVarType);
          auto zeroVal = ConstantInt::get(Type::getInt8Ty(M->getContext()), 0);
          entryBuilder.CreateMemSet(allocaInst, zeroVal, sizeInByte, 1);
        } else {
          entryBuilder.CreateStore(initializer, allocaInst);
        }
      }

      /*
       * Replace all uses of the global variable in the entry function with an
       * allocaInst. The allocaInst is placed at the beginning of
       * the entry block and is initialized with the global variable's
       * initializer.
       */
      unordered_set<Instruction *> instsToReplace;
      for (auto user : G.users()) {
        if (auto inst = dyn_cast<Instruction>(user)) {
          if (inst->getParent()->getParent() == currentF) {
            instsToReplace.insert(inst);
          }
        }
      }
      for (auto inst : instsToReplace) {
        inst->replaceUsesOfWith(&G, allocaInst);
        errs() << prefix << "Replace global variable " << globalVarName << "\n";
        errs() << emptyPrefix << "with allocaInst: " << *allocaInst << "\n";
      }
    }
  }
}

unordered_set<Function *> PrivatizerManager::getPrivatizableFunctions(
    Noelle &noelle,
    GlobalVariable *globalVar,
    PointToSummary *ptSum) {
  auto globalVarName = G.getName();

  auto isFunctionReachable = [&](Function *caller, Function *callee) {
    auto fm = noelle.getFunctionManager();
    return fm->getFunctionsReachableFrom(caller).count(callee) > 0;
  };

  unordered_set<Function *> hotFunctions;
  for (auto &[f, _] : ptSum->functionSummaries) {
    hotFunctions.insert(f);
  }

  bool isGlobalVarWritten = false;
  for (auto f : hotFunctions) {
    auto funcSum = ptSum->functionSummaries.at(f);
    auto funcPtGraph = funcSum->functionPointToGraph;
    for (auto storeInst : funcSum->storeInsts) {
      auto pointerOperand = ptSum->getVariable(storeInst->getPointerOperand());
      auto memObjsMayBeWritten = funcPtGraph->getPointees(pointerOperand);
      if (memObjsMayBeWritten.count(globalVar) > 0) {
        isGlobalVarWritten = true;
        break;
      }
    }
  }
  if (!isGlobalVarWritten) {
    errs() << prefix << "Global variable " << globalVarName
           << " is never written, no need to privatize it.\n";
    return {};
  }

  unordered_set<Function *> privatizableFunctions;
  for (auto user : G.users()) {
    if (auto inst = dyn_cast<Instruction>(user)) {
      auto userFunction = inst->getParent()->getParent();
      if (hotFunctions.count(userFunction) > 0) {
        privatizableFunctions.insert(userFunction);
      }
    }
  }

  if (privatizableFunctions.empty()) {
    return privatizableFunctions;
  } else if (privatiableFuncions.size() == 1) {
    auto currentF = *privatizableFunctions.begin();
    auto funcSum = ptSum->functionSummaries.at(currentF);
    auto globalMemObj = ptSum->getMemoryObject(globalVar);

    if (funcSum->reachableFromReturnValue.count(globalMemObj) > 0) {
      return {};
    } else if (isFunctionReachable(currentF, currentF)) {
      return {};
    } else {
      return privatizableFunctions;
    }
  } else {
    auto privatizableCandiates = privatizableFunctions;
    for (auto funcA : privatizableCandidates) {
      for (auto funcB : privatizableCandidates) {
        if (isFunctionReachable(funcA, funcB)
            || isFunctionReachable(funcB, funcA)) {
          privatizableFunctions.erase(funcA);
          privatizableFunctions.erase(funcB);
        }
      }
    }

    for (auto currentF : privatizableFunctions) {
      auto funcSum = ptSum->functionSummaries.at(currentF);
      auto globalMemObj = ptSum->getMemoryObject(globalVar);

      if (funcSum->reachableFromReturnValue.count(globalMemObj) > 0) {
        return {};
      } else if (globalVariableInitializedInFunction(globalVar, currentF)) {
        return {};
      }
    }

    return privatizableFunctions;
  }
}

bool PrivatizerManager::globalVariableInitializedInFunction(
    Noelle &noelle,
    GlobalVariable *globalVar,
    Function *currentF) {
  unordered_set<StoreInst *> initCandidates;
  unordered_set<Instruction *> users;

  auto funcSum = ptSum->functionSummaries.at(currentF);
  auto funcPtGraph = funcSum->functionPointToGraph;

  for (auto storeInst : funcSum->storeInsts) {
    auto pointerOperand = ptSum->getVariable(storeInst->getPointerOperand());
    auto memObjsMayBeWritten = funcPtGraph->getPointees(pointerOperand);
    if (memObjsMayBeWritten.count(globalVar) > 0) {
      initCandidates.insert(storeInst);
      users.insert(storeInst);
    }
  }

  for (auto loadInst : funcSum->loadInsts) {
    auto pointerOperand = ptSum->getVariable(loadInst->getPointerOperand());
    auto memObjsMayBeRead = funcPtGraph->getPointees(pointerOperand);
    if (memObjsMayBeRead.count(globalVar) > 0) {
      users.insert(loadInst);
    }
  }

  for (auto callInst : funcSum->callInsts) {
    for (auto &arg : callInst->arg_operands()) {
      auto operand = ptSum->getVariable(
          strip(callInst->getArgOperand(arg.getOperandNo())));
      auto memObjsMayBeAccessed = funcPtGraph->getPointees(operand);
      if (memObjsMayBeAccessed.count(globalVar) > 0) {
        users.insert(callInst);
      }
    }
  }

  unordered_set<StoreInst *> storeInsts = initCandidates;

  auto getProgramPointOfInitilization = [&](StoreInst *storeInst)
      -> Instruction * {
    if (globalVar->getValueType()->isArrayTy()) {
      auto loopStructures = noelle.getLoopStructures();
      auto loop = noelle.getLoop(LS);
      auto loopToInitArray = loop->getNestedMostLoopStructure(storeInst);
      auto IVM = loop->getInductionVariableManager();
      auto GIV = IVM->getLoopGoverningInductionVariable();

      if (GIV == nullptr) {
        return nullptr;
      }

      LLVMContext &C = storeInst->getContext();
      auto arrayType = dyn_cast<ArrayType>(globalVar->getValueType());
      auto arraySize =
          ConstantInt::get(Type::getInt32Ty(C), arrayType->getNumElements());

      auto IV = GIV->getInductionVariable();
      auto startValue = IV->getStartValue();
      auto stepValue = IV->getSingleComputedStepValue();
      auto exitConditionValue = GIV->getExitConditionValue();

      auto startFromZero =
          (startValue == ConstantInt::get(Type::getInt32Ty(C), 0));
      auto stepIsOne = (stepValue == ConstantInt::get(Type::getInt32Ty(C), 1));
      auto exitArraySize = (exitConditionValue == arraySize);

      if (startFromZero && stepIsOne && exitArraySize) {
        return loopToInitArray->getHeader()->getTerminator();
      } else {
        return nullptr;
      }
    } else {
      return storeInst;
    }
  }

         auto dominateAllUsers = [&](StoreInst *storeInst) {
           auto DS = noelle.getDominators(currentF);
           for (auto user : users) {
             if ((storeInst == user) || DS.DT.dominates(storeInst, user)) {
               continue;
             } else {
               return false;
             }
           }
           return true;
         } for (auto storeInst : storeInsts) {
    if (!dominateAllUsers(storeInst)) {
      initCandidates.erase(storeInst);
    }
  }

  if (initCandidates.empty()) {
    return false;
  }

  assert(initCandidates.size() == 1);
  auto initCandidate = *initCandidates.begin();

  auto valueOperand = ptSum->getVariable(initCandidate->getValueOperand());
  auto memObjsUsedAsInitializer = funcPtGraph->getPointees(valueOperand);
  if (!intersect(memObjsUsedAsInitializer, globalMemoryObjects).empty()) {
    return false;
  } else if (memObjsUsedAsInitializer.count(ptSum->unknownMemoryObject) > 0) {
    return false;
  }
}

} // namespace llvm::noelle
