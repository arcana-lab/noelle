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
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/ADT/iterator_range.h"
#include <set>
#include <unordered_map>

#include "DGBase.hpp"
#include "SCC.hpp"
#include "PDG.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  /*
   * SCCDAG of a loop.
   */
  class SCCDAG : public DG<SCC> {
    public:

      /*
       * Constructor.
       */
      SCCDAG (PDG *loopDependenceGraph) ;

      /*
       * Check if @inst is included in the SCCDAG.
       */
      bool doesItContain (Instruction *inst) const ;

      /*
       * Return the number of instructions that compose the SCCDAG.
       */
      int64_t numberOfInstructions (void) ;

      /*
       * Iterate over SCCs until @funcToInvoke returns true or no other SCC exists.
       */
      bool iterateOverSCCs (std::function<bool (SCC *)> funcToInvoke);

      /*
       * Iterate over instructions inside the SCCDAG until @funcToInvoke returns true or no other instruction exists.
       */
      bool iterateOverInstructions (std::function<bool (Instruction *)> funcToInvoke);

      /*
       * Iterate over live-ins and live-outs of the loop represented by the SCCDAG until @funcToInvoke returns true or no other live-in and live-out exist.
       */
      bool iterateOverLiveInAndLiveOut (std::function<bool (Instruction *)> funcToInvoke);

      /*
       * Iterate over all instructions (internal and external) until @funcToInvoke returns true or no other instruction exists.
       * External nodes represent live-ins and live-outs of the loop represented by the SCCDAG.
       */
      bool iterateOverAllInstructions (std::function<bool (Instruction *)> funcToInvoke);

      /*
       * Merge SCCs of @sccSet to become a single node of the SCCDAG.
       */
      void mergeSCCs (std::set<DGNode<SCC> *> &sccSet);

      /*
       * Return the SCC that contains @val
       */
      SCC * sccOfValue (Value *val) const;

      /*
       * Deconstructor.
       */
      ~SCCDAG() ;


    protected:
      void markValuesInSCC();
      void markEdgesAndSubEdges();

      unordered_map<Value *, DGNode<SCC> *> valueToSCCNode;
  };
}
