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

Pointer::Pointer(Value *source) {
  this->source = source;
}

Value *Pointer::getSource(void) {
  return source;
}

Variable::Variable(Value *source) : Pointer(source) {}

PointNodeType Variable::getType(void) {
  return PointNodeType::VARIABLE;
}

MemoryObject::MemoryObject(Value *source, bool prev) : Pointer(source) {
  this->prevLoopAllocated = prev;
};

PointNodeType MemoryObject::getType(void) {
  return PointNodeType::MEMORY_OBJECT;
}

FunctionSummary::FunctionSummary(Function *F) {
  this->F = F;
  this->M = F->getParent();
  this->basicBlockCount = 0;

  for (auto &bb : *F) {
    this->basicBlockCount++;
    for (auto &inst : bb) {
      if (isa<AllocaInst>(inst)) {
        auto allocaInst = dyn_cast<AllocaInst>(&inst);
        this->allocaInsts.insert(allocaInst);
      } else if (isa<LoadInst>(inst)) {
        auto loadInst = dyn_cast<LoadInst>(&inst);
        this->loadInsts.insert(loadInst);
      } else if (isa<StoreInst>(inst)) {
        auto storeInst = dyn_cast<StoreInst>(&inst);
        this->storeInsts.insert(storeInst);
      } else if (isa<CallBase>(inst)) {
        auto callInst = dyn_cast<CallBase>(&inst);
        auto fname = getCalledFuncName(callInst);
        if (fname == MALLOC) {
          this->mallocInsts.insert(callInst);
        } else if (fname == CALLOC) {
          this->callocInsts.insert(callInst);
        } else if (fname == REALLOC) {
          this->reallocInsts.insert(callInst);
        } else if (fname == FREE) {
          this->freeInsts.insert(callInst);
        } else if ((!isLifetimeIntrinsic(callInst))
                   && (!isa<MemCpyInst>(callInst))
                   && (READ_ONLY_LIB_FUNCTIONS_WITH_SUFFIX.count(fname) == 0)) {
          this->unknownFuntctionCalls.insert(callInst);
        }
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

  for (auto &bb : *funcSum->F) {
    bbIN[&bb] = PointToGraph();
    bbOUT[&bb] = PointToGraph();

    for (auto &inst : bb) {
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

PointToSummary::~PointToSummary() {
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

}; // namespace llvm::noelle
