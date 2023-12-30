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

namespace arcana::noelle {

UserSummary::UserSummary(GlobalVariable *globalVar, Noelle &noelle) {
  auto hotFuncs = hotFunctions(noelle);
  std::queue<User *> worklist;
  std::queue<bool> isDirectUser;
  std::unordered_map<Instruction *, std::unordered_set<User *>> inst2op;
  for (auto user : globalVar->users()) {
    worklist.push(user);
    isDirectUser.push(true);
  }

  while (!worklist.empty()) {
    auto user = worklist.front();
    auto direct = isDirectUser.front();
    worklist.pop();
    isDirectUser.pop();

    if (isa<Instruction>(user)) {
      auto inst = dyn_cast<Instruction>(user);
      auto f = inst->getFunction();
      if (hotFuncs.find(f) != hotFuncs.end()) {
        if (direct) {
          userInsts[f].insert(inst);
          users[f].insert(inst);
        } else {
          userInsts[f].insert(inst);
          for (auto op : inst2op[inst]) {
            users[f].insert(op);
          }
        }
      }
    } else if (isa<Operator>(user)) {
      auto op = dyn_cast<Operator>(user);
      for (auto opUser : op->users()) {
        if (isa<Instruction>(opUser)) {
          auto inst = dyn_cast<Instruction>(opUser);
          worklist.push(inst);
          isDirectUser.push(false);
          inst2op[inst].insert(op);
        }
      }
    }
  }

  for (auto &[f, insts] : userInsts) {
    userFunctions.insert(f);
  }
};

bool isFixedSizedHeapAllocation(CallBase *heapAllocInst) {
  auto calleeFunc = heapAllocInst->getCalledFunction();
  if (calleeFunc && calleeFunc->getName() == "malloc") {
    if (dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))) {
      return true;
    }
  } else if (calleeFunc && calleeFunc->getName() == "calloc") {
    if (dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))
        && dyn_cast<ConstantInt>(heapAllocInst->getOperand(1))) {
      return true;
    }
  }
  return false;
}

uint64_t getAllocationSize(Value *allocationSource) {
  if (isa<AllocaInst>(allocationSource)) {
    auto allocaInst = dyn_cast<AllocaInst>(allocationSource);
    auto dl = allocaInst->getModule()->getDataLayout();
    return allocaInst->getAllocationSizeInBits(dl).getValue() / 8;
  } else if (isa<GlobalVariable>(allocationSource)) {
    auto globalVar = dyn_cast<GlobalVariable>(allocationSource);
    auto globalVarType = globalVar->getValueType();
    auto dl = globalVar->getParent()->getDataLayout();
    return dl.getTypeAllocSize(globalVarType);
  } else if (isa<CallBase>(allocationSource)) {
    auto heapAllocInst = dyn_cast<CallBase>(allocationSource);
    if (isFixedSizedHeapAllocation(heapAllocInst)) {
      auto dl = heapAllocInst->getModule()->getDataLayout();
      auto calleeFunc = heapAllocInst->getCalledFunction();
      if (calleeFunc && calleeFunc->getName() == "malloc") {
        return dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))
            ->getZExtValue();
      } else if (calleeFunc && calleeFunc->getName() == "calloc") {
        auto elementCount =
            dyn_cast<ConstantInt>(heapAllocInst->getOperand(0))->getZExtValue();
        auto elementSizeInBytes =
            dyn_cast<ConstantInt>(heapAllocInst->getOperand(1))->getZExtValue();
        return elementCount * elementSizeInBytes;
      }
    }
  }
  assert(false && "Unsupported allocation source.");
};

std::unordered_set<Function *> functionsInvokedFrom(Noelle &noelle,
                                                    Function *caller) {

  auto fm = noelle.getFunctionsManager();
  auto mainF = fm->getEntryFunction();
  auto pcf = fm->getProgramCallGraph();

  auto insertMyCallees = [&](Function *caller,
                             std::queue<Function *> &funcsToTraverse) {
    auto funcNode = pcf->getFunctionNode(caller);
    for (auto callEdge : pcf->getOutgoingEdges(funcNode)) {
      for (auto subEdge : callEdge->getSubEdges()) {
        auto calleeFunc = subEdge->getCallee()->getFunction();
        if (!calleeFunc || calleeFunc->empty()) {
          continue;
        }
        funcsToTraverse.push(calleeFunc);
      }
    }
  };

  std::unordered_set<Function *> funcSet;
  std::queue<Function *> funcsToTraverse;
  insertMyCallees(caller, funcsToTraverse);

  while (!funcsToTraverse.empty()) {
    auto func = funcsToTraverse.front();
    funcsToTraverse.pop();
    if (funcSet.find(func) != funcSet.end()) {
      continue;
    }
    funcSet.insert(func);
    insertMyCallees(func, funcsToTraverse);
  }

  return funcSet;
};

std::unordered_set<Function *> hotFunctions(Noelle &noelle) {
  auto mainF = noelle.getFunctionsManager()->getEntryFunction();
  auto hotFuncs = functionsInvokedFrom(noelle, mainF);
  hotFuncs.insert(mainF);
  return hotFuncs;
}

} // namespace arcana::noelle
