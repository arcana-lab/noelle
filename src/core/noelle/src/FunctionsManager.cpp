/*
 * Copyright 2020 - 2021  Simone Campanoni
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
#include "noelle/core/FunctionsManager.hpp"

namespace llvm::noelle {

FunctionsManager::FunctionsManager(Module &m,
                                   PDGAnalysis &noellePDGAnalysis,
                                   Hot *profiles)
  : program{ m },
    pdgAnalysis{ noellePDGAnalysis },
    pcg{ nullptr },
    prof{ profiles },
    nonMemModifiersIsInitialized{ false } {
  return;
}

Function *FunctionsManager::getEntryFunction(void) const {
  auto f = this->program.getFunction("main");
  return f;
}

CallGraph *FunctionsManager::getProgramCallGraph(void) {
  if (this->pcg == nullptr) {
    this->pcg = this->pdgAnalysis.getProgramCallGraph();
  }

  return this->pcg;
}

bool FunctionsManager::isTheLibraryFunctionPure(Function *libraryFunction) {

  /*
   * Check if the function is a library function.
   */
  if (libraryFunction == nullptr) {
    return false;
  }
  if (!libraryFunction->empty()) {
    return false;
  }

  /*
   * The function is a library function.
   * Check if it is known to be pure.
   */
  if (this->pdgAnalysis.isTheLibraryFunctionPure(libraryFunction)) {
    return true;
  }

  return false;
}

bool FunctionsManager::hasStoreInst(Function &f) const {
  for (const auto &inst : f) {
    if (isa<StoreInst>(inst)) {
      return true;
    }
  }
  return false;
}

bool FunctionsManager::canModifyMemory(Function &f) {
  if (this->nonMemModifiersIsInitialized) {
    bool isMemModifier =
        this->nonMemModifiers.find(&f) == this->nonMemModifiers.end();
    return isMemModifier;
  }

  /*
   * Explore the call graph via breadth-first search going from a node
   * to its incident nodes.
   */
  std::set<CallGraphFunctionNode *> nonExplored;
  std::queue<CallGraphFunctionNode *> toTraverse;
  std::set<CallGraphFunctionNode *> enqueued;
  auto pcg = this->getProgramCallGraph();
  for (auto fn : pcg->getFunctionNodes()) {
    auto f = fn->getFunction();
    bool isUnavailable = f->empty();
    bool isModifier = this->hasStoreInst(*f);

    /*
     * For conservativeness unavailable functions are assumed to modify memory
     */
    if (isUnavailable || isModifier) {
      toTraverse.push(fn);
      enqueued.insert(fn);
    } else {
      nonExplored.insert(fn);
    }
  }

  while (!toTraverse.empty()) {
    auto fn = toTraverse.front();
    auto f = fn->getFunction();
    toTraverse.pop();
    nonExplored.erase(fn);

    /*
     * Adding incoming edges to the set of nodes to explore.
     * No caller is added twice to the queue
     */
    for (auto incomingEdge : fn->getIncomingEdges()) {
      auto callerFuncNode = incomingEdge->getCaller();
      bool notYetExplored =
          nonExplored.find(callerFuncNode) != nonExplored.end();
      if (notYetExplored) {
        auto callerFunc = callerFuncNode->getFunction();
        bool alreadyEnqueued = enqueued.find(callerFuncNode) != enqueued.end();
        if (!alreadyEnqueued) {
          toTraverse.push(callerFuncNode);
          enqueued.insert(callerFuncNode);
        }
      }
    }
  }

  /*
   * At this point, non-explored nodes are nodes that cannot be reached
   * from any node whose function call has a StoreInst, therefore they
   * represent calls to functions that cannot modify memory
   */
  for (auto fn : nonExplored) {
    auto f = fn->getFunction();
    this->nonMemModifiers.insert(f);
  }
  this->nonMemModifiersIsInitialized = true;

  bool isMemModifier =
      this->nonMemModifiers.find(&f) == this->nonMemModifiers.end();
  return isMemModifier;
}

std::set<Function *> FunctionsManager::getProgramConstructors(void) const {
  std::set<Function *> s;

  /*
   * Fetch the list of global ctors of the module.
   */
  auto globalCtor = this->program.getGlobalVariable("llvm.global_ctors");
  if (globalCtor == nullptr) {
    return s;
  }

  /*
   * Fetch the initializers.
   */
  auto init = globalCtor->getInitializer();
  assert(init != nullptr);
  auto initVector = cast<ConstantArray>(init);
  assert(initVector != nullptr);
  for (auto &V : initVector->operands()) {

    /*
     * Fetch the next constructor.
     */
    if (isa<ConstantAggregateZero>(V)) {
      continue;
    }
    auto CS = cast<ConstantStruct>(V);
    if (isa<ConstantPointerNull>(CS->getOperand(1))) {
      continue;
    }
    auto maybeFunction = CS->getOperand(1);
    if (!isa<Function>(maybeFunction)) {
      continue;
    }
    auto function = cast<Function>(maybeFunction);

    s.insert(function);
  }

  return s;
}

Function *FunctionsManager::newFunction(const std::string &name,
                                        FunctionType &signature) {

  /*
   * Get the function
   */
  auto r = this->program.getOrInsertFunction(name, &signature);
  auto newFunction = cast<Function>(r.getCallee());
  if (newFunction == nullptr) {
    errs() << "NOELLE: FunctionsManager::newFunction: ERROR = function \""
           << name << "\" cannot be created\n";
    abort();
  }

  /*
   * Check if the function existed before
   */
  if (!newFunction->empty()) {
    errs() << "NOELLE: FunctionsManager::newFunction: ERROR = function \""
           << name << "\" already existed\n";
    abort();
  }

  return newFunction;
}

Function *FunctionsManager::getFunction(const std::string &name) {
  auto f = this->program.getFunction(name);

  return f;
}

std::set<Function *> FunctionsManager::getFunctions(void) const {
  std::set<Function *> s;

  for (auto &f : this->program) {
    s.insert(&f);
  }

  return s;
}

std::set<Function *> FunctionsManager::getFunctionsReachableFrom(
    Function *startingPoint) {
  std::set<Function *> functions;

  /*
   * Fetch the call graph.
   */
  auto callGraph = this->getProgramCallGraph();

  /*
   * Compute the set of functions reachable from the starting point.
   */
  std::set<Function *> funcSet;
  std::queue<Function *> funcToTraverse;
  funcToTraverse.push(startingPoint);
  while (!funcToTraverse.empty()) {
    auto func = funcToTraverse.front();
    funcToTraverse.pop();
    if (funcSet.find(func) != funcSet.end())
      continue;
    funcSet.insert(func);

    auto funcCGNode = callGraph->getFunctionNode(func);
    for (auto outEdge : funcCGNode->getOutgoingEdges()) {
      auto calleeNode = outEdge->getCallee();
      auto F = calleeNode->getFunction();
      if (!F) {
        continue;
      }
      if (F->empty()) {
        continue;
      }
      funcToTraverse.push(F);
    }
  }

  /*
   * Iterate over functions of the module and add to the vector only the ones
   * that are reachable from the starting point. This will enforce that the
   * order of the functions returned follows the one of the module.
   */
  for (auto &f : this->program) {
    if (funcSet.find(&f) == funcSet.end()) {
      continue;
    }
    functions.insert(&f);
  }

  return functions;
}

void FunctionsManager::sortByHotness(std::vector<Function *> &functions) {

  /*
   * Define the order between functions.
   */
  auto compareLoops = [this](Function *f1, Function *f2) -> bool {
    auto aInsts = this->prof->getTotalInstructions(f1);
    auto bInsts = this->prof->getTotalInstructions(f2);
    return aInsts > bInsts;
  };

  /*
   * Sort the functions.
   */
  std::sort(functions.begin(), functions.end(), compareLoops);

  return;
}

void FunctionsManager::removeFunction(Function &f) {
  f.eraseFromParent();
}

} // namespace llvm::noelle
