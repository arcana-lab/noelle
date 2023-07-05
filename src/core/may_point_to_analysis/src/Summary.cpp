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
  auto updatedPtes = unite(oldPtes, newPtes);

  if (oldPtes != updatedPtes) {
    ptGraph[ptr] = updatedPtes;
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
    if (pointer->getType() == PointNodeType::MEMORY_OBJECT) {
      reachable.insert((MemoryObject *)ptr);
    }
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

Pointers PointToGraph::getAllPointers(void) {
  Pointers allPointers;
  for (auto &[ptr, ptes] : ptGraph) {
    allPointers.insert(ptr);
  }
  return allPointers;
}

FunctionSummary::FunctionSummary(Function *currentF) {
  this->currentF = currentF;
  this->functionPointToGraph = new PointToGraph();

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
        this->callInsts.insert(callInst);
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
            break;
          case USER_DEFINED:
          case UNKNOWN:
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

MemoryObjects FunctionSummary::memoryObjectsCanBeAccessedAfterReturn() {
  MemoryObjects result;
  for (auto ptr : functionPointToGraph->getAllPointers()) {
    if (ptr->getType() == PointNodeType::VARIABLE) {
      assert(ptr->getSource() != nullptr && "Pointer source is null");
      auto inst = dyn_cast<Instruction>(ptr->getSource());
      if (inst && inst->getParent()->getParent() == currentF) {
        // The pointer is a variable defined in the current function
        // do nothing
      } else {
        // The pointer is a variable defined in other functions
        // or a global variable
        result =
            unite(result, functionPointToGraph->getReachableMemoryObjects(ptr));
      }
    }
  }
  return unite(result, memoryObjectsReachableFromReturnValue());
}

MemoryObjects FunctionSummary::memoryObjectsReachableFromReturnValue() {
  MemoryObjects result;
  for (auto memObj : returnMemoryObjects) {
    result =
        unite(result, functionPointToGraph->getReachableMemoryObjects(memObj));
  }
  return result;
}

PointToSummary::PointToSummary(Module &M) : M(M) {
  for (auto &G : M.globals()) {
    auto globalVar = getVariable(&G);
    auto globalMemObj = getMemoryObject(&G);
    globalMemoryObjects.insert(globalMemObj);
  }
  unknownMemoryObject = new MemoryObject(NULL);
  memoryObjects[NULL] = unknownMemoryObject;
}

PointToSummary::~PointToSummary() {
  for (auto &[_, funcSum] : functionSummaries) {
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
}

Variable *PointToSummary::getVariable(Value *source) {
  assert(source != nullptr && "Source cannot be null");
  auto strippedValue = isa<Instruction>(source) ? strip(source) : source;
  if (variables.count(strippedValue) == 0) {
    variables[strippedValue] = new Variable(strippedValue);
  }
  return variables[strippedValue];
}

MemoryObject *PointToSummary::getMemoryObject(Value *source) {
  assert(source != nullptr && "Source cannot be null");
  auto strippedValue = isa<Instruction>(source) ? strip(source) : source;
  if (memoryObjects.count(strippedValue) == 0) {
    memoryObjects[strippedValue] = new MemoryObject(strippedValue);
  }
  return memoryObjects[strippedValue];
}

FunctionSummary *PointToSummary::getFunctionSummary(Function *function) {
  if (functionSummaries.count(function) == 0) {
    functionSummaries[function] = new FunctionSummary(function);
  }
  return functionSummaries[function];
}

}; // namespace llvm::noelle
