/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni, Brian Homerding
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "Assumptions.h"
#include "noelle/core/DominatorSummary.hpp"
#include "noelle/core/DGBase.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/StayConnectedNestedLoopForest.hpp"

namespace llvm::noelle {

  class LoopCarriedDependencies {
    public:
      static void setLoopCarriedDependencies (StayConnectedNestedLoopForestNode *loopNode, const DominatorSummary &DS, PDG &dgForLoops) ;

      static std::set<DGEdge<Value> *> getLoopCarriedDependenciesForLoop (const LoopStructure &LS, StayConnectedNestedLoopForestNode *loopNode, PDG &LoopDG) ;
      static std::set<DGEdge<Value> *> getLoopCarriedDependenciesForLoop (const LoopStructure &LS, StayConnectedNestedLoopForestNode *loopNode, SCCDAG &sccdag);

    private:
      static LoopStructure * getLoopOfLCD(StayConnectedNestedLoopForestNode *loopNode, const DominatorSummary &DS, DGEdge<Value> *edge) ;

      static bool canBasicBlockReachHeaderBeforeOther (const LoopStructure &LS, BasicBlock *I, BasicBlock *J) ;

  };

}
