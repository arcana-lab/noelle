/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/LoopEnvironment.hpp"

namespace llvm::noelle { 

LoopEnvironment::LoopEnvironment (
  PDG *loopDG, 
  std::vector<BasicBlock *> &exitBlocks
  ) : LoopEnvironment(loopDG, exitBlocks, {})
{
  return ;
}

LoopEnvironment::LoopEnvironment (
  PDG *loopDG, 
  std::vector<BasicBlock *> &exitBlocks,
  const std::set<Value *> &excludeValues
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
     * Skip values that need to be excluded
     */
    if (excludeValues.find(externalValue) != excludeValues.end()){
      continue ;
    }

    /*
     * Determine whether the external value is a producer (i.e., live-in).
     */
    auto isProducer = false;
    std::unordered_set<Instruction *> consumersOfLiveInValue;
    for (auto edge : externalNode->getOutgoingEdges()) {

      /*
       * Memory and control dependences can be skipped as they do not dictate live-in values.
       */
      if (edge->isMemoryDependence() || edge->isControlDependence()) {
        continue;
      }

      /*
       * The current dependence from @externalNode to an instruction within a loop means we have a new live-in value.
       */
      isProducer = true;

      /*
       * Fetch the current consumer of the new live-in value.
       */
      auto consumerOfNewLiveIn = edge->getIncomingT();
      assert(isa<Instruction>(consumerOfNewLiveIn));
      auto consumerOfNewLiveIn_inst = cast<Instruction>(consumerOfNewLiveIn);

      /*
       * Add the current consumer of the new live-in we have just found.
       */
      consumersOfLiveInValue.insert(consumerOfNewLiveIn_inst);
    }
    if (isProducer) {
      this->addLiveInValue(externalValue, consumersOfLiveInValue);
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

Type * LoopEnvironment::typeOfEnvironmentLocation (uint64_t index) const {
  if (index < envProducers.size()) {
    return envProducers[index]->getType();
  }

  return exitBlockType;
}

uint64_t LoopEnvironment::addProducer (Value *producer, bool liveIn){

  /*
   * Make sure @producer isn't already part of the environment.
   */
  uint64_t c = 0;
  for (auto p : this->envProducers){
    if (p == producer) {
      assert(this->envProducers[c] == producer);
      return c;
    }
    c++;
  }

  /*
   * Add @producer to the environment.
   */
  auto envIndex = this->envProducers.size();
  this->envProducers.push_back(producer);
  producerIndexMap[producer] = envIndex;
  if (liveIn) {
    liveInInds.insert(envIndex);
  } else {
    liveOutInds.insert(envIndex);
  }

  return envIndex;
}
      
uint64_t LoopEnvironment::addLiveInValue (Value *newLiveInValue, const std::unordered_set<Instruction *> &consumers){

  /*
   * Add the live-in value.
   */
  auto newIndex = this->addLiveInProducer(newLiveInValue);

  /*
   * Add the consumers.
   */
  for (auto consumerOfNewLiveIn : consumers){
    this->prodConsumers[newLiveInValue].insert(consumerOfNewLiveIn);
  }

  return newIndex;
}

bool LoopEnvironment::isProducer (Value *producer) const {
  return producerIndexMap.find(producer) != producerIndexMap.end();
}

bool LoopEnvironment::isLiveIn (Value *val) const {

  /*
   * Check if @val belongs to the environment.
   */
  if (producerIndexMap.find(val) == producerIndexMap.end()){
    return false;
  }

  /*
   * Fetch the index of @val.
   */
  auto indexOfVal = producerIndexMap.at(val);

  /*
   * Check if @val is a live-in.
   */
  auto isLiveIn = isProducer(val) && liveInInds.find(indexOfVal) != liveInInds.end();

  return isLiveIn;
}

uint64_t LoopEnvironment::addLiveInProducer (Value *producer) { 
  auto newIndex = addProducer(producer, true); 
  return newIndex;
}

void LoopEnvironment::addLiveOutProducer (Value *producer) { 
  addProducer(producer, false); 
  return ;
}

int64_t LoopEnvironment::indexOfExitBlockTaken (void) const {
  return hasExitBlockEnv ? envProducers.size() : -1; 
}

uint64_t LoopEnvironment::size (void) const {
  return envProducers.size() + (hasExitBlockEnv ? 1 : 0);
}

std::set<Value *> LoopEnvironment::consumersOf (Value *prod) const {
  std::set<Value *> s;
  if (prodConsumers.find(prod) == prodConsumers.end()){
    return s;
  }

  s = prodConsumers.at(prod);
  return s;
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

Value * LoopEnvironment::producerAt (uint64_t ind) const { 
  assert(ind < this->envProducers.size());
  return envProducers[ind];
}

}
