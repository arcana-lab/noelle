/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DeadFunctionEliminator.hpp"

namespace llvm::noelle {

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

}
