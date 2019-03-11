/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/AssumptionCache.h"

#include "LoopDependenceInfo.hpp"
#include "Queue.hpp"

using namespace llvm;

namespace llvm {

  enum class Verbosity { Disabled, Minimal, Maximal };

  struct Parallelization : public ModulePass {
    public:

      /*
       * Fields.
       */
      static char ID;
      IntegerType *int1, *int8, *int16, *int32, *int64;
      Queue queues;

      /*
       * Methods.
       */
      Parallelization();

      virtual ~Parallelization();

      bool doInitialization (Module &M) override ;

      void getAnalysisUsage(AnalysisUsage &AU) const override ;

      bool runOnModule (Module &M) override ;

      std::vector<Function *> * getModuleFunctionsReachableFrom (
        Module *module, 
        Function *startingPoint
        );

      std::vector<LoopDependenceInfo *> * getModuleLoops (
        Module *module, 
        double minimumHotness
        );

      uint32_t getNumberOfModuleLoops (
        Module *module,
        double minimumHotness
        );

      void linkParallelizedLoopToOriginalFunction (
        Module *module, 
        BasicBlock *originalPreHeader, 
        BasicBlock *startOfParLoopInOriginalFunc,
        BasicBlock *endOfParLoopInOriginalFunc,
        Value *envArray,
        Value *envIndexForExitVariable,
        SmallVector<BasicBlock *, 10> &loopExitBlocks
        );

    private:
      uint32_t fetchTheNextValue (
        std::stringstream &stream
        );

      bool filterOutLoops (
        char *fileName,
        std::vector<uint32_t>& loopThreads,
        std::vector<uint32_t>& techniquesToDisable,
        std::vector<uint32_t>& DOALLChunkSize
        );
  };
}
