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

MemoryObjects PointToGraph::reachableMemoryObjects(Pointer *pointer) {
  MemoryObjects reachable;
  set<Pointer *> worklist;
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

FunctionSummary::FunctionSummary(Function *caller, Function *currentF) {
  this->caller = caller;
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
        auto calledFunc = callInst->getCalledFunction();
        auto fname = getCalledFuncName(callInst);
        if (fname == MALLOC) {
          this->mallocInsts.insert(callInst);
        } else if (fname == CALLOC) {
          this->callocInsts.insert(callInst);
        } else if (fname == REALLOC) {
          this->reallocInsts.insert(callInst);
        } else if (fname == FREE) {
          this->freeInsts.insert(callInst);
        } else if (isLifetimeIntrinsic(callInst)) {
          // DO NOTHING
        } else if (isa<MemCpyInst>(callInst)) {
          // DO NOTHING
        } else if (READ_ONLY_LIB_FUNCTIONS_WITH_SUFFIX.count(fname) > 0) {
          // DO NOTHING
        } else if (calledFunc != nullptr && !calledFunc->isDeclaration()) {
          // DO NOTHING
          // A user-defined function is not unknown,
          // its point-to info will be stored in another function summary
        } else {
          this->unknownFuntctionCalls.insert(callInst);
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

  set<FunctionCall> funcInvokes;
  auto mainF = M.getFunction("main");
  auto islands = callGraph->getIslands();
  auto islandOfMain = islands[mainF];

  for (auto node : callGraph->getFunctionNodes()) {
    auto f = node->getFunction();
    /*
     * An empty function or function with only declaration will not call our
     * function
     */
    if (f->empty() || f->isDeclaration()) {
      continue;
    }
    /*
     * Ignore cold functions
     */
    if (islands[f] != islandOfMain) {
      continue;
    }

    for (auto callEdge : node->getOutgoingEdges()) {
      /*
       * funcInvokes cares only about the must call edges
       * for may call edges (i.e. call with function pointer), we can not get
       * the callee function we will assume the most conservative case later in
       * may_point_to_analysis.
       */
      if (callEdge->isAMustCall()) {
        auto calleeNode = callEdge->getCallee();
        auto calleeF = calleeNode->getFunction();
        funcInvokes.insert(make_pair(f, calleeF));
      }
    }
  }

  funcSums[make_pair(nullptr, mainF)] = new FunctionSummary(nullptr, mainF);
  for (auto &[callerF, calleeF] : funcInvokes) {
    auto funcSum = new FunctionSummary(callerF, calleeF);
    funcSums[make_pair(callerF, calleeF)] = funcSum;

    for (auto inst : funcSum->mallocInsts) {
      getVariable(inst);
      getMemoryObject(inst);
    }

    for (auto inst : funcSum->callocInsts) {
      getVariable(inst);
      getMemoryObject(inst);
    }

    for (auto inst : funcSum->allocaInsts) {
      getVariable(inst);
      getMemoryObject(inst);
    }

    for (auto &arg : funcSum->currentF->args()) {
      getVariable(&arg);
    }
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
