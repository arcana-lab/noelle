/*
 * Copyright 2019 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/PDG.hpp"
#include "noelle/core/StayConnectedNestedLoopForest.hpp"

namespace llvm::noelle {

  class InvariantManager {
    public:
      InvariantManager (
        LoopStructure *loop,
        PDG *loopDG
      );
      
      InvariantManager () = delete;

      bool isLoopInvariant (Value *value) const ;

      std::unordered_set<Instruction *> getLoopInstructionsThatAreLoopInvariants (void) const ;

    private:
      std::unordered_set<Instruction *> invariants;
      LoopStructure *ls;

      /*
       * This inner class defines methods to determine whether values are invariant
       * and relies on the dependence graph passed to the invariant manager. However,
       * that graph may become invalidated later on, so to prevent keeping that pointer
       * around after the construction of InvariantManager, we encapsulate it in this inner class
       */ 
      class InvarianceChecker {
        public:

          InvarianceChecker (LoopStructure *loop, PDG *loopDG, std::unordered_set<Instruction *> &invariants) ;

        private:
          LoopStructure *loop;
          PDG *loopDG;
          std::unordered_set<Instruction *> &invariants;

          /*
           * Used to cache instructions already checked and known NOT to be invariant
           */
          std::unordered_set<Instruction *> notInvariants;

          /*
           * To pass as a lambda to a dependence iteration function on PDG,
           * we bind isEvolvingValue to this member variable
           */
          bool isEvolvingValue (Value *toValue, DGEdge<Value> *dep) ;

          /*
           * For each top level call to isEvolvingValue, track values checked
           * In case of a cycle of dependencies
           */
          std::unordered_set<Instruction *> dependencyValuesBeingChecked;

          bool arePHIIncomingValuesEquivalent (PHINode *phi) ;

      };

  };

}
