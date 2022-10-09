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

FunctionsManager::FunctionsManager(Module &m, PDGAnalysis &noellePDGAnalysis)
  : program{ m },
    pdgAnalysis{ noellePDGAnalysis },
    pcg{ nullptr } {
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

void FunctionsManager::removeFunction(Function &f) {
  f.eraseFromParent();
}

} // namespace llvm::noelle
