/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DeadFunctionEliminator.hpp"

using namespace llvm;
using namespace llvm::noelle;

DeadFunctionEliminator::DeadFunctionEliminator ()
  :
  ModulePass{ID}
  {
  return ;
}

bool DeadFunctionEliminator::runOnModule (Module &M) {
  auto modified = false;

  /*
    * Fetch the outputs of the passes we rely on.
    */
  auto& noelle = getAnalysis<Noelle>();

  /*
    * Fetch the call graph.
    */
  auto pcg = noelle.getProgramCallGraph();

  /*
    * Check if there are functions with only one caller.
    * Inline them.
    */
  for (auto node : pcg->getFunctionNodes()){

    /*
      * Fetch the node.
      */
    auto nodeFunction = node->getFunction();

    /*
      * Check the function has a body.
      */
    if (nodeFunction->empty()){
      continue ;
    }

    /*
      * Check how many other functions can invoke @nodeFunction
      */
    auto callerNodes = node->getIncomingEdges();
    if (callerNodes.size() != 1){
      continue ;
    }

    /*
      * We found a function that has only one other function that can invoke it.
      *
      * Check how many callers this function includes.
      */
    auto callingEdge = *callerNodes.begin();
    auto callerEdges = callingEdge->getSubEdges();
    if (callerEdges.size() != 1) {
      continue ;
    }

    /*
      * @node has only one caller.
      *
      * Fetch the caller.
      */
    auto callerEdge = *callerEdges.begin();
    auto callerNode = callerEdge->getCaller();
    auto callerInst = callerNode->getInstruction();

    /*
      * Check the single caller isn't part of the same function of @node.
      */
    if (callerInst->getFunction() == nodeFunction){
      continue ;
    }

    /*
      * Check the instruction isn't an invoke.
      */
    if (isa<InvokeInst>(callerInst)){

      /*
        * We cannot inline invoke instructions to preserve the exception handling mechanisms.
        */
      continue ;
    }

    /*
      * We can inline @nodeFunction without increasing the size of the binary.
      *
      * Check if we need to translate an indirect call to a direct call.
      */
    auto callInst = cast<CallInst>(callerInst);
    if (callInst->getCalledFunction() == nullptr){

      /*
        * This is an indirect call.
        * Translate it to a direct call.
        */
      //TODO
      continue ;
    }
    assert(callInst->getCalledFunction() == nodeFunction);
    InlineFunctionInfo IFI;
    errs() << "DeadFunctionEliminator: Inline " << *callInst << " into " << callInst->getFunction()->getName() << "\n";
    //modified |= InlineFunction(callInst, IFI);
  }
  if (modified) {
    return true;
  }

  /*
    * Fetch the islands.
    */
  auto islands = pcg->getIslands();

  /*
    * Fetch the island of the entry method of the program.
    */
  auto entryF = noelle.getEntryFunction();
  auto entryIsland = islands[entryF];
  std::unordered_set<CallGraph *> liveIslands{entryIsland};

  /*
    * Fetch the islands of all constructors.
    */
  auto globalCtor = M.getGlobalVariable("llvm.global_ctors");
  if (globalCtor != nullptr){
    auto init = globalCtor->getInitializer();
    assert(init != nullptr);
    auto initVector = cast<ConstantArray>(init);
    assert(initVector != nullptr);
    for (auto &V : initVector->operands()){

      /*
        * Fetch the next constructor.
        */
      if (isa<ConstantAggregateZero>(V)){
        continue;
      }
      auto CS = cast<ConstantStruct>(V);
      if (isa<ConstantPointerNull>(CS->getOperand(1))){
        continue;
      }
      auto maybeFunction = CS->getOperand(1);
      if (!isa<Function>(maybeFunction)){
        continue ;
      }
      auto function = cast<Function>(maybeFunction);
      errs() << "DeadFunctionEliminator:  Considering ctor " << function->getName() << " as entry function\n";

      /*
        * Fetch the island.
        */
      auto ctorIsland = islands[function];
      assert(ctorIsland != nullptr);
      liveIslands.insert(ctorIsland);
    }
  }

  /*
    * Delete dead functions.
    */
  std::vector<Function *>toDelete;
  for (auto &F : M){
    if (F.isIntrinsic()){
      continue ;
    }
    if (F.empty()){
      continue ;
    }
    auto n = pcg->getFunctionNode(&F);
    if (liveIslands.find(islands[&F]) == liveIslands.end()){
      errs() << "DeadFunctionEliminator: Function " << F.getName() << " is dead\n";
      toDelete.push_back(&F);
    }
  }
  for (auto f : toDelete){
    f->eraseFromParent();
    modified = true;
  }

  return modified;
}
