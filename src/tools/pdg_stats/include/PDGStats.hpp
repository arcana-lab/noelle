/*
 * Copyright 2016 - 2020  Yian Su
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/IR/Module.h"

using namespace llvm;

namespace llvm {
  
  struct PDGStats : public ModulePass {
    public:
      enum EDGE_ATTRIBUTE {
        IS_MEMORY_DEPENDENCE = 2,
        IS_MUST_DEPENDENCE,
        DATA_DEPENDENCE,
        IS_CONTROL_DEPENDENCE,
        IS_LOOP_CARRIED_DEPENDENCE,
        IS_REMOVABLE_DEPENDENCE
      };
      static char ID;

      PDGStats();
      virtual ~PDGStats();
      bool doInitialization(Module &M) override;
      void getAnalysisUsage(AnalysisUsage &AU) const override;
      bool runOnModule(Module &M) override;
    
    private:
      int64_t numberOfNodes = 0;
      int64_t numberOfEdges = 0;
      int64_t numberOfVariableDependence = 0;
      int64_t numberOfMemoryDependence = 0;
      int64_t numberOfMemoryMustDependence = 0;
      int64_t numberOfPotentialMemoryDependences = 0;
      int64_t numberOfControlDependence = 0;

      void collectStatsForNodes(Function &F);
      void collectStatsForPotentialEdges (Function &F) ;
      void collectStatsForEdges(Function &F);
      bool edgeIsDependenceOf(MDNode *edgeM, EDGE_ATTRIBUTE edgeAttribute);
      void printStats();
  };

}
