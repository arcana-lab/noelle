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
#include <unordered_set>

#include "DGBase.hpp"
#include "SCC.hpp"
#include "PDG.hpp"
#include "BitMatrix.hpp"

using namespace std;
using namespace llvm;
using namespace llvm::noelle;

namespace llvm::noelle {

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
       * Return the range that includes all SCCs.
       */
      std::unordered_set<SCC *> getSCCs (void) ;

      /*
       * Iterate over instructions inside the SCCDAG until @funcToInvoke returns true or no other instruction exists.
       */
      bool iterateOverInstructions (std::function<bool (Instruction *)> funcToInvoke);

      /*
       * Iterate over live-ins and live-outs of the loop represented by the SCCDAG until @funcToInvoke returns true or no other live-in and live-out exist.
       */
      bool iterateOverLiveInAndLiveOut (std::function<bool (Value *)> funcToInvoke);

      /*
       * Iterate over all instructions (internal and external) until @funcToInvoke returns true or no other instruction exists.
       * External nodes represent live-ins and live-outs of the loop represented by the SCCDAG.
       */
      bool iterateOverAllInstructions (std::function<bool (Instruction *)> funcToInvoke);

      /*
       * Iterate over all values (internal and external) until @funcToInvoke returns true or no other value exists.
       * External values represent live-ins and live-outs of the loop represented by the SCCDAG.
       */
      bool iterateOverAllValues (std::function<bool (Value *)> funcToInvoke);

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

      /*
       * Define a collection of SCCs type.
       */
      typedef std::vector<SCC *> SCCSet;

      /*
       * Compute transitive dependences between nodes of the SCCDAG.
       */
      void computeReachabilityAmongSCCs();

      /*
       * Compute ordering between nodes of the SCCDAG.
       */
      bool orderedBefore(const SCC *earlySCC, const SCCSet &lates) const;
      bool orderedBefore(const SCCSet &earlies, const SCC *lateSCC) const;
      bool orderedBefore(const SCC *earlySCC, const SCC *lateSCC) const;

      /*
       * Get the index of a node of the SCCDAG.
       */
      uint32_t getSCCIndex(const SCC *scc) const;

    protected:
      void markValuesInSCC (void);
      void markEdgesAndSubEdges (void);

      unordered_map<Value *, DGNode<SCC> *> valueToSCCNode;

    private:

      /*
       * BitMatrix for keeping the topological order of the SCCDAG nodes.
       */
      BitMatrix ordered;

      /*
       * ordered_dirty is true if the ordering of SCCDAG nodes is invalid.
       */
      bool orderedDirty;

      // SCC nodes to Ids map.
      std::unordered_map<const SCC *, uint32_t> sccIndexes;
  };
}
