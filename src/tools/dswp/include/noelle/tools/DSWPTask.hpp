/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef NOELLE_SRC_TOOLS_DSWP_DSWPTASK_H_
#define NOELLE_SRC_TOOLS_DSWP_DSWPTASK_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/Task.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCC.hpp"

using namespace std;

namespace arcana::noelle {

struct QueueInfo;
struct QueueInstrs;

class DSWPTask : public Task {
public:
  /*
   * Constructor
   */
  DSWPTask(FunctionType *taskSignature, Module &M);

  /*
   * DSWP specific task function arguments
   */
  Value *queueArg;

  /*
   * Original loops' relevant structures
   */
  std::set<SCC *> stageSCCs;
  std::set<SCC *> clonableSCCs;

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
};

struct QueueInfo {
  int fromStage, toStage;
  Type *dependentType;
  int bitLength;
  bool isMemoryDependence;

  Instruction *producer;
  std::set<Instruction *> consumers;
  unordered_map<Instruction *, int> consumerToPushIndex;

  QueueInfo(Instruction *p, Instruction *c, Type *type, bool isMemoryDependence)
    : producer{ p },
      dependentType{ type },
      isMemoryDependence{ isMemoryDependence } {
    consumers.insert(c);
    if (isMemoryDependence) {
      dependentType = IntegerType::get(c->getContext(), 1);
      bitLength = 1;
    } else if (dependentType->isPointerTy()) {
      bitLength =
          DataLayout(p->getModule()).getTypeAllocSize(dependentType) * 8;
    } else {
      bitLength = dependentType->getPrimitiveSizeInBits();
      // NOTE(angelo): Round up to the nearest power of 2
      bitLength = pow(2, ceil(log2(bitLength)));
    }
  }

  raw_ostream &print(raw_ostream &stream, std::string prefixToUse = "") {
    producer->print(stream << prefixToUse << "From stage: " << fromStage
                           << " To stage: " << toStage << " Number of bits: "
                           << bitLength << " Producer: ");
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
} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_DSWP_DSWPTASK_H_
