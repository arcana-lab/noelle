/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
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
#include "noelle/core/LoopEnvironment.hpp"

namespace llvm::noelle {

LoopEnvironment::LoopEnvironment(PDG *loopDG,
                                 std::vector<BasicBlock *> &exitBlocks)
  : LoopEnvironment(loopDG, exitBlocks, {}) {
  return;
}

LoopEnvironment::LoopEnvironment(PDG *loopDG,
                                 std::vector<BasicBlock *> &exitBlocks,
                                 const std::set<Value *> &excludeValues) {

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
    if (excludeValues.find(externalValue) != excludeValues.end()) {
      continue;
    }

    /*
     * Determine whether the external value is a producer (i.e., live-in).
     */
    auto isProducer = false;
    std::unordered_set<Instruction *> consumersOfLiveInValue;
    for (auto edge : externalNode->getOutgoingEdges()) {

      /*
       * Memory and control dependences can be skipped as they do not dictate
       * live-in values.
       */
      if (edge->isMemoryDependence() || edge->isControlDependence()) {
        continue;
      }

      /*
       * The current dependence from @externalNode to an instruction within a
       * loop means we have a new live-in value.
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
   * In this case, we need an extra variable to keep track of which exit has
   * been taken.
   */
  this->hasExitBlockEnv = exitBlocks.size() > 1;
  if (this->hasExitBlockEnv) {
    auto &cxt = exitBlocks[0]->getContext();
    this->exitBlockType = IntegerType::get(cxt, 32);
  }

  return;
}

Type *LoopEnvironment::typeOfEnvironmentLocation(uint64_t id) const {
  if (id < envProducers.size()) {
    return this->envProducers[id]->getType();
  }

  return exitBlockType;
}

std::vector<Type *> LoopEnvironment::getTypesOfEnvironmentLocations(
    void) const {

  /*
   * Collect the Type of each environment variable
   */
  std::vector<Type *> varTypes;
  for (auto i = 0u; i < this->size(); ++i) {
    varTypes.push_back(this->typeOfEnvironmentLocation(i));
  }

  return varTypes;
}

uint64_t LoopEnvironment::addProducer(Value *producer, bool liveIn) {

  /*
   * Make sure @producer isn't already part of the environment.
   */
  uint64_t c = 0;
  for (auto p : this->envProducers) {
    if (p == producer) {
      assert(this->envProducers[c] == producer);
      return c;
    }
    c++;
  }

  /*
   * Add @producer to the environment.
   */
  auto nextEnvID = this->envProducers.size();
  this->envProducers.push_back(producer);
  producerIDMap[producer] = nextEnvID;
  if (liveIn) {
    liveInIDs.insert(nextEnvID);
  } else {
    liveOutIDs.insert(nextEnvID);
  }

  return nextEnvID;
}

uint64_t LoopEnvironment::addLiveInValue(
    Value *newLiveInValue,
    const std::unordered_set<Instruction *> &consumers) {

  /*
   * Add the live-in value.
   */
  auto newID = this->addLiveInProducer(newLiveInValue);

  /*
   * Add the consumers.
   */
  for (auto consumerOfNewLiveIn : consumers) {
    this->prodConsumers[newLiveInValue].insert(consumerOfNewLiveIn);
  }

  return newID;
}

bool LoopEnvironment::isProducer(Value *producer) const {
  return producerIDMap.find(producer) != producerIDMap.end();
}

bool LoopEnvironment::isLiveIn(Value *val) const {
  assert(val != nullptr);

  /*
   * Check if @val belongs to the environment.
   */
  if (producerIDMap.find(val) == producerIDMap.end()) {
    return false;
  }

  /*
   * Fetch the id of @val.
   */
  auto producerID = producerIDMap.at(val);

  /*
   * Check if @val is a live-in.
   */
  auto isLiveIn =
      isProducer(val) && liveInIDs.find(producerID) != liveInIDs.end();

  return isLiveIn;
}

uint64_t LoopEnvironment::addLiveInProducer(Value *producer) {
  auto newID = this->addProducer(producer, true);
  return newID;
}

void LoopEnvironment::addLiveOutProducer(Value *producer) {
  this->addProducer(producer, false);
  return;
}

int64_t LoopEnvironment::getExitBlockID(void) const {
  return this->hasExitBlockEnv ? this->envProducers.size() : -1;
}

uint64_t LoopEnvironment::size(void) const {
  return envProducers.size() + (hasExitBlockEnv ? 1 : 0);
}

std::set<Value *> LoopEnvironment::consumersOf(Value *prod) const {
  std::set<Value *> s;
  if (prodConsumers.find(prod) == prodConsumers.end()) {
    return s;
  }

  s = prodConsumers.at(prod);
  return s;
}

iterator_range<std::vector<Value *>::iterator> LoopEnvironment::getProducers(
    void) {
  return make_range(envProducers.begin(), envProducers.end());
}

iterator_range<std::set<int>::iterator> LoopEnvironment::getEnvIDsOfLiveInVars(
    void) {
  return make_range(liveInIDs.begin(), liveInIDs.end());
}

iterator_range<std::set<int>::iterator> LoopEnvironment::getEnvIDsOfLiveOutVars(
    void) {
  return make_range(liveOutIDs.begin(), liveOutIDs.end());
}

Value *LoopEnvironment::getProducer(uint64_t id) const {
  assert(id < this->envProducers.size());
  return envProducers[id];
}

} // namespace llvm::noelle
