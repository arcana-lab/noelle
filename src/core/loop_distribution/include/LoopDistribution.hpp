/*
 * Copyright 2019 - 2020  Lukas Gross, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "LoopDependenceInfo.hpp"
#include "LoopStructure.hpp"
#include "SCC.hpp"

namespace llvm::noelle {

  class LoopDistribution {
    public:

      /*
       * Methods
       */
      LoopDistribution();

      bool splitLoop (
        LoopDependenceInfo const &LDI, 
        SCC *SCCToPullOut,
        std::set<Instruction *> &instructionsRemoved,
        std::set<Instruction *> &instructionsAdded
        );

      bool splitLoop (
        LoopDependenceInfo const &LDI, 
        std::set<SCC *> const &SCCsToPullOut,
        std::set<Instruction *> &instructionsRemoved,
        std::set<Instruction *> &instructionsAdded
        );

    private:

      /*
       * Fields
       */

      /*
       * Methods
       */
      bool splitLoop (
        LoopDependenceInfo const &LDI,
        std::set<Instruction *> &instsToPullOut,
        std::set<Instruction *> &instructionsRemoved,
        std::set<Instruction *> &instructionsAdded
      );

      void recursivelyCollectDependencies (
        Instruction * inst,
        std::set<Instruction *> &toPopulate,
        LoopDependenceInfo const &LDI
      );

      bool splitWouldBeTrivial (
        LoopStructure * const loopStructure,
        std::set<Instruction *> const &instsToPullOut,
        std::set<Instruction *> const &instsToClone
      );

      bool splitWouldRequireForwardingDataDependencies (
        LoopDependenceInfo const &LDI,
        std::set<Instruction *> const &instsToPullOut,
        std::set<Instruction *> const &instsToClone
      );

      void doSplit (
        LoopDependenceInfo const &LDI,
        std::set<Instruction *> const &instsToPullOut,
        std::set<Instruction *> const &instsToClone,
        std::set<Instruction *> &instructionsRemoved,
        std::set<Instruction *> &instructionsAdded
      );

  };

}
