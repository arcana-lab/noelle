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

#include "noelle/core/CallGraph.hpp"
#include "noelle/core/MayPointToAnalysis.hpp"
#include "MayPointToAnalysisUtils.hpp"

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

MemoryObject::MemoryObject(Value *source) : Pointer(source) {}

PointNodeType MemoryObject::getType(void) {
  return PointNodeType::MEMORY_OBJECT;
}

PointToGraph::PointToGraph() {}

/*
 * Returns the set of memory objects that the pointer may point to.
 * The pointer can be a variable or a memory object
 */
MemoryObjects PointToGraph::getPointees(Pointer *pointer) {
  return (ptGraph.count(pointer) > 0) ? ptGraph[pointer] : MemoryObjects();
}

bool PointToGraph::setPointees(Pointer *ptr, MemoryObjects newPtes) {
  auto oldPtes = getPointees(ptr);
  if (oldPtes != newPtes) {
    ptGraph[ptr] = newPtes;
    return true;
  } else {
    return false;
  }
}

bool PointToGraph::addPointees(Pointer *ptr, MemoryObjects newPtes) {
  auto oldPtes = getPointees(ptr);
  if (oldPtes != newPtes) {
    ptGraph[ptr] = unite(oldPtes, newPtes);
    return true;
  } else {
    return false;
  }
}

MemoryObjects PointToGraph::getReachableMemoryObjects(Pointer *pointer) {
  MemoryObjects reachable;
  unordered_set<Pointer *> worklist;
  worklist.insert(pointer);
  while (!worklist.empty()) {
    auto ptr = *worklist.begin();
    worklist.erase(ptr);
    for (auto pte : getPointees(ptr)) {
      if (reachable.count(pte) == 0) {
        reachable.insert(pte);
        worklist.insert(pte);
      }
    }
  }
  return reachable;
};

MemoryObject *PointToGraph::mustPointToMemory(Pointer *pointer) {
  auto ptes = getPointees(pointer);
  if (ptes.size() == 0 || ptes.size() > 1) {
    return nullptr;
  }
  auto memoryObject = dyn_cast<MemoryObject>(*ptes.begin());
  return memoryObject;
}

FunctionSummary::FunctionSummary(Function *currentF) {
  this->currentF = currentF;

  for (auto &bb : *currentF) {
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
      } else if (isa<ReturnInst>(inst)) {
        auto returnInst = dyn_cast<ReturnInst>(&inst);
        this->returnInsts.insert(returnInst);
      } else if (isa<CallBase>(inst)) {
        auto callInst = dyn_cast<CallBase>(&inst);
        auto functionType = getMPAFunctionType(callInst);
        switch (functionType) {
          case MALLOC:
            this->mallocInsts.insert(callInst);
            break;
          case CALLOC:
            this->callocInsts.insert(callInst);
            break;
          case REALLOC:
            this->reallocInsts.insert(callInst);
            break;
          case FREE:
            this->freeInsts.insert(callInst);
            break;
          case INTRINSIC:
          case READ_ONLY:
          case MEM_COPY:
          case USER_DEFINED:
            break;
          case UNKNOWN:
            this->unknownFuntctionCalls.insert(callInst);
            break;
        }
      }
    }
  }
}

FunctionSummary::~FunctionSummary() {
  if (functionPointToGraph != nullptr) {
    delete functionPointToGraph;
  }
}

PointToSummary::PointToSummary(Module &M, CallGraph *callGraph) : M(M) {
  for (auto &G : M.globals()) {
    auto globalVar = new Variable(&G);
    auto globalMemObj = new MemoryObject(&G);
    variables[&G] = globalVar;
    memoryObjects[&G] = globalMemObj;
    globalMemoryObjects.insert(globalMemObj);
  }
}

PointToSummary::~PointToSummary() {
  for (auto &[_, funcSum] : funcSums) {
    if (funcSum != nullptr) {
      delete funcSum;
    }
  }
  for (auto &[_, variable] : variables) {
    if (variable != nullptr) {
      delete variable;
    }
  }
  for (auto &[_, memoryObject] : memoryObjects) {
    if (memoryObject != nullptr) {
      delete memoryObject;
    }
  }
  for (auto globalMemoryObject : globalMemoryObjects) {
    if (globalMemoryObject != nullptr) {
      delete globalMemoryObject;
    }
  }
  if (unknownMemoryObject != nullptr) {
    delete unknownMemoryObject;
  }
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

FunctionSummary *PointToSummary::getFunctionSummary(Function *function) {
  if (funcSums.count(function) == 0) {
    funcSums[function] = new FunctionSummary(function);
  }
  return funcSums[function];
}

}; // namespace llvm::noelle
