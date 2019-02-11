/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include <unordered_map>
#include <vector>
#include <set>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "PDG.hpp"

namespace llvm {

  class LoopEnvironment {
    public:
      LoopEnvironment (PDG *loopDG, SmallVector<BasicBlock *, 10> &exitBlocks);

      iterator_range<std::vector<Value *>::iterator> getProducers() { 
        return make_range(envProducers.begin(), envProducers.end());
      }

      iterator_range<std::set<int>::iterator> getEnvIndicesOfLiveInVars() { 
        return make_range(liveInInds.begin(), liveInInds.end());
      }

      iterator_range<std::set<int>::iterator> getEnvIndicesOfLiveOutVars() { 
        return make_range(liveOutInds.begin(), liveOutInds.end());
      }

      /*
       * One per external dependent + one to track exit block
       */
      int envSize (void) const ;

      int indexOfExitBlock (void) const ;

      Type *typeOfEnv (int index) const ;

      bool isLiveIn (Value *val);

      inline Value *producerAt (int ind) { return envProducers[ind]; }

      std::set<Value *> &consumersOf (Value *prod);

      bool isProducer (Value *producer) const ;

    private:
      void addLiveInProducer (Value *producer);
      void addLiveOutProducer (Value *producer);
      void addProducer (Value *producer, bool liveIn);

      /*
       * DEPRECATED(angelo): use of this API suggests poor environment 
       * algorithm, as all producers should just be iterated over for
       * any arbitrary operation
       */
      int indexOfProducer (Value *producer) { return producerIndexMap[producer]; }

      std::vector<Value *> envProducers;
      std::unordered_map<Value *, int> producerIndexMap;

      std::set<int> liveInInds;
      std::set<int> liveOutInds;

      std::unordered_map<Value *, std::set<Value *>> prodConsumers;

      bool hasExitBlockEnv;
      Type *exitBlockType;
  };

}
