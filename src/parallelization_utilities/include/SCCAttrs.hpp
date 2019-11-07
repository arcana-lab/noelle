/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "SCC.hpp"

namespace llvm {

  class SCCAttrs {
    public:

      /*
       * Fields
       */
      std::set<BasicBlock *> bbs;
      std::set<Value *> stronglyConnectedDataValues;
      std::set<Value *> weaklyConnectedDataValues;
      bool isClonable;
      bool hasIV;

      std::set<PHINode *> PHINodes;
      std::set<Instruction *> accumulators;
      PHINode *singlePHI;
      Instruction *singleAccumulator;
      std::set<std::pair<Value *, Instruction *>> controlPairs;

      /*
       * Constructor
       */
      SCCAttrs (SCC *s);

      /*
       * Get the SCC.
       */
      SCC * getSCC (void);

      void collectSCCValues ();

      const std::pair<Value *, Instruction *> * getSingleInstructionThatControlLoopExit (void);

    private:
      SCC *scc;
      std::set<Instruction *> controlFlowInsts;
  
      void collectControlFlowInstructions (void);
  };

}
