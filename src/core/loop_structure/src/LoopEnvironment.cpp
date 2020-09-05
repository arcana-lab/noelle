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
  std::vector<BasicBlock *> &exitBlocks
  ) {

  /*
   * Initialize the environment of the loop.
   */
  for (auto nodeI : loopDG->externalNodePairs()) {

    /*
     * Fetch the live in/out variable.
     */
    auto externalNode = nodeI.second;
    auto externalValue = externalNode->getT();

    /*
     * Determine whether the external value is a producer (i.e., live-in).
     */
    auto isProducer = false;
    for (auto edge : externalNode->getOutgoingEdges()) {
      if (edge->isMemoryDependence() || edge->isControlDependence()) {
        continue;
      }
      isProducer = true;
      this->prodConsumers[externalValue].insert(edge->getIncomingT());
    }
    if (isProducer) {
      this->addLiveInProducer(externalValue);
    }

    /*
     * Determine whether the external value is a consumer (i.e., live-out).
     */
    for (auto edge : externalNode->getIncomingEdges()) {
      if (edge->isMemoryDependence() || edge->isControlDependence()) {
        continue;
      }
      auto internalValue = edge->getOutgoingT();
      if (!this->isProducer(internalValue)) {
        this->addLiveOutProducer(internalValue);
      }
      this->prodConsumers[internalValue].insert(externalValue);
    }
  }

  /*
   * Check if there are multiple exits for this loop.
   * In this case, we need an extra variable to keep track of which exit has been taken.
   */
  this->hasExitBlockEnv = exitBlocks.size() > 1;
  if (this->hasExitBlockEnv) {
    auto &cxt = exitBlocks[0]->getContext();
    this->exitBlockType = IntegerType::get(cxt, 32);
  }

  return ;
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

  return ;
}

bool LoopEnvironment::isProducer (Value *producer) const {
  return producerIndexMap.find(producer) != producerIndexMap.end();
}

bool LoopEnvironment::isLiveIn (Value *val) {
  return isProducer(val) && liveInInds.find(producerIndexMap[val]) != liveInInds.end();
}

void LoopEnvironment::addLiveInProducer (Value *producer) { 
  addProducer(producer, true); 
  return ;
}

void LoopEnvironment::addLiveOutProducer (Value *producer) { 
  addProducer(producer, false); 
  return ;
}

int LoopEnvironment::indexOfExitBlock (void) const {
  return hasExitBlockEnv ? envProducers.size() : -1; 
}

int LoopEnvironment::envSize (void) const {
  return envProducers.size() + (hasExitBlockEnv ? 1 : 0);
}

std::set<Value *> LoopEnvironment::consumersOf (Value *prod) {
  return prodConsumers[prod];
}

iterator_range<std::vector<Value *>::iterator> LoopEnvironment::getProducers (void) { 
  return make_range(envProducers.begin(), envProducers.end());
}
      
iterator_range<std::set<int>::iterator> LoopEnvironment::getEnvIndicesOfLiveInVars (void) { 
  return make_range(liveInInds.begin(), liveInInds.end());
}
      
iterator_range<std::set<int>::iterator> LoopEnvironment::getEnvIndicesOfLiveOutVars (void) { 
  return make_range(liveOutInds.begin(), liveOutInds.end());
}

Value * LoopEnvironment::producerAt (uint32_t ind) { 
  return envProducers[ind]; 
}
