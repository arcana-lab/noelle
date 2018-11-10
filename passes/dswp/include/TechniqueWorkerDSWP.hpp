#pragma once

#include <math.h>

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "TechniqueWorker.hpp"
#include "PDG.hpp"
#include "SCC.hpp"

#include <unordered_map>

using namespace std;

namespace llvm {

  struct QueueInfo;
  struct QueueInstrs;

  struct DSWPTechniqueWorker : TechniqueWorker {
    DSWPTechniqueWorker ()
      : stageSCCs{}, removableSCCs{}, usedCondBrs{} {}

    /*
     * DSWP specific worker function arguments
     */
    Value *queueArg;

    /*
     * Original loops' relevant structures
     */
    std::set<SCC *> stageSCCs;
    std::set<SCC *> removableSCCs;
    std::set<TerminatorInst *> usedCondBrs;

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
