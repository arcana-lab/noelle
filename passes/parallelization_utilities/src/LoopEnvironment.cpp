/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopEnvironment.hpp"

using namespace llvm ;

LoopEnvironment::LoopEnvironment (
  PDG *loopDG, 
  SmallVector<BasicBlock *, 10> &exitBlocks
  ) {

  for (auto nodeI : loopDG->externalNodePairs()) {

    /*
     * Fetch the live out variable.
     */
    auto externalNode = nodeI.second;
    auto externalValue = externalNode->getT();

    /*
     * Determine whether the external value is a producer
     */
    bool isProducer = false;
    for (auto edge : externalNode->getOutgoingEdges()) {
      if (edge->isMemoryDependence() || edge->isControlDependence()) continue;
      isProducer = true;
      this->prodConsumers[externalValue].insert(edge->getIncomingT());
    }
    if (isProducer) {
      this->addLiveInProducer(externalValue);
    }

    /*
     * Determine whether the external value is a consumer
     */
    for (auto edge : externalNode->getIncomingEdges()) {
      if (edge->isMemoryDependence() || edge->isControlDependence()) continue;
      auto internalValue = edge->getOutgoingT();
      if (!this->isProducer(internalValue)) {
        this->addLiveOutProducer(internalValue);
      }
      this->prodConsumers[internalValue].insert(externalValue);
    }
  }

  this->hasExitBlockEnv = exitBlocks.size() > 1;
  if (this->hasExitBlockEnv) {
    this->exitBlockType = IntegerType::get(exitBlocks[0]->getContext(), 32);
  }
}

Type * LoopEnvironment::typeOfEnv (int index) const {
  if (index < envProducers.size()) {
    return envProducers[index]->getType();
  }

  return exitBlockType;
}

void LoopEnvironment::addProducer (Value *producer, bool liveIn){
  auto envIndex = envProducers.size();
  envProducers.push_back(producer);
  producerIndexMap[producer] = envIndex;
  if (liveIn) {
    liveInInds.insert(envIndex);
  } else {
    liveOutInds.insert(envIndex);
  }
}

bool LoopEnvironment::isProducer (Value *producer) const {
  return producerIndexMap.find(producer) != producerIndexMap.end();
}

bool LoopEnvironment::isLiveIn (Value *val) {
  return isProducer(val) && liveInInds.find(producerIndexMap[val]) != liveInInds.end();
}

void LoopEnvironment::addLiveInProducer (Value *producer) { 
  addProducer(producer, true); 
}

void LoopEnvironment::addLiveOutProducer (Value *producer) { 
  addProducer(producer, false); 
}

int LoopEnvironment::indexOfExitBlock (void) const {
  return hasExitBlockEnv ? envProducers.size() : -1; 
}

int LoopEnvironment::envSize (void) const {
  return envProducers.size() + (hasExitBlockEnv ? 1 : 0);
}

std::set<Value *> &LoopEnvironment::consumersOf (Value *prod) {
  return prodConsumers[prod];
}
