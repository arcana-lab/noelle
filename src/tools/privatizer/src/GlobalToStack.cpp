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
#include "Privatizer.hpp"
#include "Utils.hpp"

namespace llvm::noelle {

bool Privatizer::applyG2S(Noelle &noelle) {
  bool modified = false;
  for (auto &[globalVar, privariableFunctions] : collectG2S(noelle)) {
    modified |= transformG2S(noelle, globalVar, privariableFunctions);
  }
  clearFunctionSummaries();
  return modified;
}

unordered_set<Function *> Privatizer::getPrivatizableFunctions(
    Noelle &noelle,
    GlobalVariable *globalVar) {

  if (globalVar->isConstant()) {
    return {};
  }

  auto mayInvoke = [&](Function *caller, Function *callee) -> bool {
    return functionsInvokedFrom(noelle, caller).count(callee) > 0;
  };

  auto privatizable = UserSummary(globalVar, noelle).userFunctions;
  assert(!privatizable.empty());

  /*
   * Each function that uses the global variable may read or write the global
   * variable. Therefore, the design choice is to check whether each user
   * funciton will never read data written by another user function.
   *
   * If so, we could privatize the global variable to all user functions
   * separately. If not, then we do nothing.
   *
   * To check each user function will never read data written by another one,
   * each user function must satisfy three conditions:
   *
   * 1. The global variable should be initialized before any use.
   * 2. The global variable should not be used by my callee.
   * 3. The global variable should not be pointed by other global variables,
   *    arguments of the current func, and return values of the current func.
   *
   * The first condition says the current user function will not read data
   * written by user functions invoked before the current user function.
   *
   * The second condition says other user functions will not directly write
   * data to the global variable during the current user function.
   *
   * The third condition says the global variable will never be written through
   * pointers during the current function.
   */
  if (privatizable.size() == 1) {
    auto currentF = *privatizable.begin();
    auto funcSum = getFunctionSummary(currentF);

    if (mayInvoke(currentF, currentF)) {
      return {};
    } else if (!initializedBeforeAllUse(noelle, globalVar, currentF)) {
      return {};
    } else if (mpa.notPrivatizable(globalVar, currentF)
               || funcSum->isDestOfMemcpy(globalVar)) {
      return {};
    }
    return privatizable;
  } else {
    for (auto funcA : privatizable) {
      for (auto funcB : privatizable) {
        if (mayInvoke(funcA, funcB)) {
          return {};
        }
      }
    }

    for (auto currentF : privatizable) {
      auto funcSum = getFunctionSummary(currentF);
      if (!initializedBeforeAllUse(noelle, globalVar, currentF)) {
        return {};
      }
    }

    for (auto currentF : privatizable) {
      auto funcSum = getFunctionSummary(currentF);
      if (mpa.notPrivatizable(globalVar, currentF)
          || funcSum->isDestOfMemcpy(globalVar)) {
        return {};
      }
    }

    return privatizable;
  }
}

bool Privatizer::initializedBeforeAllUse(Noelle &noelle,
                                         GlobalVariable *globalVar,
                                         Function *currentF) {

  auto funcSum = getFunctionSummary(currentF);
  auto initCandidates = funcSum->storeInsts;
  auto userInsts = UserSummary(globalVar, noelle).userInsts[currentF];

  /*
   * The global variable should be initialized before all use.
   *
   * We try to find find a store instruction that will write the whole
   * memory object of the global variable, initializers refer to the
   * instructions that will initialize the global variable.
   *
   * Each user of the global variable should either be part of initializers,
   * or be dominated by the initialization. Otherwise, the global variable
   * is not initialized before all use.
   */
  auto initDominateAllUsers = [&](StoreInst *storeInst) {
    auto DS = noelle.getDominators(currentF);
    unordered_set<Instruction *> initializers;
    auto initProgramPoint =
        getInitProgramPoint(noelle, globalVar, storeInst, initializers);
    if (!initProgramPoint) {
      return false;
    }

    for (auto user : userInsts) {
      if (initializers.count(user) > 0
          || DS->DT.dominates(initProgramPoint, user)) {
        continue;
      } else {
        return false;
      }
    }
    return true;
  };

  unordered_set<StoreInst *> notInitCandidates;
  for (auto storeInst : initCandidates) {
    if (!initDominateAllUsers(storeInst)) {
      notInitCandidates.insert(storeInst);
    }
  }

  for (auto storeInst : notInitCandidates) {
    initCandidates.erase(storeInst);
  }

  if (initCandidates.empty()) {
    return false;
  }

  assert(initCandidates.size() == 1);

  return true;
}

/*
 * Check whether the storeInst is used to initialize the global variable or not.
 * If so, try to find the program point where the global variable is
 * initialized. If not, return nullptr.
 */
Instruction *Privatizer::getInitProgramPoint(
    Noelle &noelle,
    GlobalVariable *globalVar,
    StoreInst *storeInst,
    unordered_set<Instruction *> &initializers) {

  auto globalVarType = globalVar->getValueType();
  auto pointer = storeInst->getPointerOperand();

  if (globalVarType->isSingleValueType() && pointer == globalVar) {
    initializers = { storeInst };
    return storeInst;
  } else if (globalVarType->isPointerTy()) {
    if (pointer == globalVar) {
      initializers = { storeInst };
      return storeInst;
    } else if (isa<BitCastOperator>(pointer)) {
      auto bitCast = dyn_cast<BitCastOperator>(pointer);
      if (bitCast->stripPointerCasts() == globalVar) {
        initializers = { storeInst };
        return storeInst;
      }
    } else if (isa<BitCastInst>(pointer)) {
      auto bitCast = dyn_cast<BitCastInst>(pointer);
      if (bitCast->stripPointerCasts() == globalVar) {
        initializers = { bitCast, storeInst };
        return storeInst;
      }
    }
  } else if (globalVar->getValueType()->isArrayTy()) {
    /*
     * If the global variable is single value type or pointer type, then
     * it could be initialized by one store instruction.
     *
     * However, if the global variable is array type, then it should be
     * initialized in a loop. Here we try to check that the loop traverses
     * each element of the global array and writes the element.
     *
     * The first instruction of the exitnode of the loop is the program
     * point where the global variable is initialized.
     */
    auto globalGEP = dyn_cast<GetElementPtrInst>(pointer);
    if (!globalGEP || globalGEP->getPointerOperand() != globalVar) {
      return nullptr;
    }

    LoopDependenceInfo *LDI = nullptr;
    for (auto ldi : *noelle.getLoops(storeInst->getFunction())) {
      if (!ldi->getLoopStructure()->isIncluded(storeInst)) {
        continue;
      }
      if (!LDI) {
        LDI = ldi;
      } else if (LDI->getLoopStructure()->getNestingLevel()
                 < ldi->getLoopStructure()->getNestingLevel()) {
        LDI = ldi;
      }
    }
    if (!LDI) {
      return nullptr;
    }

    auto IVM = LDI->getInductionVariableManager();
    auto GIV = IVM->getLoopGoverningInductionVariable();

    if (GIV == nullptr) {
      return nullptr;
    }

    LLVMContext &C = noelle.getProgramContext();
    auto arrayType = dyn_cast<ArrayType>(globalVar->getValueType());
    auto arraySize =
        ConstantInt::get(Type::getInt32Ty(C), arrayType->getNumElements());

    auto IV = GIV->getInductionVariable();
    auto startValue = IV->getStartValue();
    auto stepValue = IV->getSingleComputedStepValue();
    auto exitCondition = GIV->getExitConditionValue();

    auto startFromZero = isa<ConstantInt>(startValue)
                         && dyn_cast<ConstantInt>(startValue)->isZero();
    auto stepIsOne = isa<ConstantInt>(stepValue)
                     && dyn_cast<ConstantInt>(stepValue)->isOne();
    auto exitArraySize = isa<ConstantInt>(exitCondition)
                         && dyn_cast<ConstantInt>(exitCondition)
                                ->equalsInt(arrayType->getNumElements());

    if (!(startFromZero && stepIsOne && exitArraySize)) {
      return nullptr;
    }

    bool GIVisIndex = false;
    for (auto &index : globalGEP->indices()) {
      if (isa<ConstantInt>(index)) {
        continue;
      } else if (index == IV->getLoopEntryPHI()) {
        GIVisIndex = true;
        continue;
      }
      return nullptr;
    }

    if (!GIVisIndex) {
      return nullptr;
    }

    auto exitNodes = LDI->getLoopStructure()->getLoopExitBasicBlocks();
    if (exitNodes.size() != 1) {
      return nullptr;
    }
    auto exitNode = *exitNodes.begin();

    initializers = { globalGEP, storeInst };
    return exitNode->getFirstNonPHI();
  }

  return nullptr;
};

unordered_map<GlobalVariable *, unordered_set<Function *>> Privatizer::
    collectG2S(Noelle &noelle) {

  unordered_map<GlobalVariable *, unordered_set<Function *>> result;

  for (auto &G : M->globals()) {

    auto globalVarName = G.getName();
    if (G.isConstant()) {
      errs() << prefix << "Global variable @" << globalVarName
             << " is constant, no need to privatize it.\n";
      continue;
    }

    if (UserSummary(&G, noelle).userFunctions.empty()) {
      errs() << prefix << "Global variable @" << globalVarName
             << " is not used, no need to privatize it.\n";
      continue;
    }

    auto privatizable = getPrivatizableFunctions(noelle, &G);
    if (privatizable.empty()) {
      errs() << prefix << "Global variable @" << globalVarName
             << " can't be privatized to any function.\n";
      continue;
    }

    result[&G] = privatizable;
  }

  return result;
}

bool Privatizer::transformG2S(Noelle &noelle,
                              GlobalVariable *globalVar,
                              unordered_set<Function *> privatizable) {
  bool modified = false;

  unordered_map<Function *, unordered_set<Instruction *>> directUses;
  unordered_map<Function *,
                unordered_map<BitCastOperator *, unordered_set<Instruction *>>>
      indirectUses;

  for (auto currentF : privatizable) {
    auto allocationSize = getAllocationSize(globalVar);
    auto funcSum = getFunctionSummary(currentF);
    auto suffix = " in function " + currentF->getName() + "\n";
    auto globalVarName = globalVar->getName();

    /*
     * Check if the stack of current function can hold the memory object of
     * the global variable. If the global variable is too large for the stack,
     * it should not be privatized to allocaInst.
     */
    if (!funcSum->stackCanHoldNewAlloca(allocationSize)) {
      errs()
          << prefix << "Stack memory usage exceeds the limit, can't privatize "
          << "global variable @" << globalVarName << suffix;
      return false;
    }

    auto usersToReplace = UserSummary(globalVar, noelle).users[currentF];
    assert(!usersToReplace.empty());

    unordered_set<Instruction *> instUsers;
    unordered_map<BitCastOperator *, unordered_set<Instruction *>> user2insts;
    for (auto user : usersToReplace) {
      if (isa<Instruction>(user)) {
        instUsers.insert(dyn_cast<Instruction>(user));
      } else if (isa<BitCastOperator>(user)) {
        auto bitCast = dyn_cast<BitCastOperator>(user);
        for (auto userOfBitCast : user->users()) {
          if (isa<Instruction>(userOfBitCast)) {
            auto inst = dyn_cast<Instruction>(userOfBitCast);
            if (inst->getFunction() == currentF) {
              user2insts[bitCast].insert(dyn_cast<Instruction>(userOfBitCast));
            }
          } else {
            errs() << prefix << "Unexpected user of global variable @"
                   << globalVarName << suffix;
            return false;
          }
        }
      } else {
        errs() << prefix << "Unexpected user of global variable @"
               << globalVarName << suffix;
        return false;
      }
    }
    directUses[currentF] = instUsers;
    indirectUses[currentF] = user2insts;
  }

  for (auto currentF : privatizable) {
    auto funcSum = getFunctionSummary(currentF);
    auto suffix = "in function " + currentF->getName() + "\n";
    auto globalVarName = globalVar->getName();

    modified = true;
    auto &context = noelle.getProgramContext();
    auto &entryBlock = currentF->getEntryBlock();
    IRBuilder<> entryBuilder(entryBlock.getFirstNonPHI());
    Type *globalVarType = globalVar->getValueType();
    AllocaInst *allocaInst =
        entryBuilder.CreateAlloca(globalVarType, nullptr, globalVarName);

    /*
     * Replace all uses of the global variable in the entry function with an
     * allocaInst. The allocaInst is placed at the beginning of the entry block.
     */
    for (auto inst : directUses[currentF]) {
      inst->replaceUsesOfWith(globalVar, allocaInst);
    }
    for (auto &[bitCastOP, insts] : indirectUses[currentF]) {
      auto destTy = bitCastOP->getDestTy();
      auto bitCastInst = entryBuilder.CreateBitCast(allocaInst, destTy);
      for (auto inst : insts) {
        inst->replaceUsesOfWith(bitCastOP, bitCastInst);
      }
    }

    errs() << prefix << "Replace global variable @" << globalVarName << "\n";
    errs() << emptyPrefix << "with allocaInst: " << *allocaInst << "\n";
    errs() << emptyPrefix << suffix;
  }

  return modified;
}

} // namespace llvm::noelle
