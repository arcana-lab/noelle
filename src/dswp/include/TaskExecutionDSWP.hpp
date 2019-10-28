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
#include "TaskExecution.hpp"
#include "PDG.hpp"
#include "SCC.hpp"

using namespace std;

namespace llvm {

  struct QueueInfo;
  struct QueueInstrs;

  class DSWPTask : public Task {
    public:
      DSWPTask ()
        : stageSCCs{}, removableSCCs{}, usedCondBrs{} {}

      /*
       * DSWP specific task function arguments
       */
      Value *queueArg;

      /*
       * Original loops' relevant structures
       */
      std::set<SCC *> stageSCCs;
      std::set<SCC *> removableSCCs;
      std::set<Instruction *> usedCondBrs;

      /*
       * Maps from producer to the queues they push to
       */
      unordered_map<Instruction *, std::set<int>> producerToQueues;

      /*
       * Maps from other stage's producer to this stage's queues
       */
      unordered_map<Instruction *, int> producedPopQueue;

      /*
       * Stores queue indices and pointers for the stage
       */
      std::set<int> pushValueQueues, popValueQueues;

      /*
       * Stores information on queue/env usage within stage
       */
      unordered_map<int, std::unique_ptr<QueueInstrs>> queueInstrMap;

      void extractFuncArgs () {
        auto argIter = this->F->arg_begin();
        this->envArg = (Value *) &*(argIter++);
        this->queueArg = (Value *) &*(argIter++);
        instanceIndexV = ConstantInt::get(
          IntegerType::get(F->getContext(), 64),
          order
        );
      }
  };

  struct QueueInfo {
    int fromStage, toStage;
    Type *dependentType;
    int bitLength;

    Instruction * producer;
    std::set<Instruction *> consumers;
    unordered_map<Instruction *, int> consumerToPushIndex;

    QueueInfo(Instruction *p, Instruction *c, Type *type)
        : producer{p}, dependentType{type} {
      consumers.insert(c);
      if (dependentType->isPointerTy()) {
        bitLength = DataLayout(p->getModule()).getTypeAllocSize(dependentType) * 8;
      } else {
        bitLength = dependentType->getPrimitiveSizeInBits();
        // NOTE(angelo): Round up to the nearest power of 2
        bitLength = pow(2, ceil(log2(bitLength)));
      }
    }

    raw_ostream &print (raw_ostream &stream, std::string prefixToUse = "") {
      producer->print(stream << prefixToUse
        << "From stage: " << fromStage << " To stage: " << toStage
        << " Number of bits: " << bitLength << " Producer: ");
      return stream << "\n";
    }
  };

  struct QueueInstrs {
    Value *queuePtr;
    Value *queueCall;
    Value *alloca;
    Value *allocaCast;
    Value *load;
  };
}
