/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_LOOP_ENVIRONMENT_LOOPENVIRONMENT_H_
#define NOELLE_SRC_CORE_LOOP_ENVIRONMENT_LOOPENVIRONMENT_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/PDG.hpp"

namespace arcana::noelle {

class LoopEnvironment {
public:
  LoopEnvironment(PDG *loopDG, std::vector<BasicBlock *> &exitBlocks);

  LoopEnvironment(PDG *loopDG,
                  std::vector<BasicBlock *> &exitBlocks,
                  const std::set<Value *> &excludeValues);

  LoopEnvironment() = delete;

  iterator_range<std::vector<Value *>::iterator> getProducers(void);

  iterator_range<std::set<int>::iterator> getEnvIDsOfLiveInVars(void) const;

  iterator_range<std::set<int>::iterator> getEnvIDsOfLiveOutVars(void) const;

  /*
   * One per live-in variables + one per live-out variable + one to track the
   * exit block (if needed)
   */
  uint64_t size(void) const;

  uint64_t getNumberOfLiveIns(void) const;

  uint64_t getNumberOfLiveOuts(void) const;

  int64_t getExitBlockID(void) const;

  Type *typeOfEnvironmentLocation(uint64_t id) const;

  std::vector<Type *> getTypesOfEnvironmentLocations(void) const;

  bool isLiveIn(Value *val) const;

  Value *getProducer(uint64_t id) const;

  std::set<Value *> consumersOf(Value *prod) const;

  bool isProducer(Value *producer) const;

  uint64_t addLiveInValue(Value *newLiveInValue,
                          const std::unordered_set<Instruction *> &consumers);

private:
  uint64_t addLiveInProducer(Value *producer);
  void addLiveOutProducer(Value *producer);
  uint64_t addProducer(Value *producer, bool liveIn);

  std::vector<Value *> envProducers;
  std::unordered_map<Value *, int> producerIDMap;

  std::set<int> liveInIDs;
  std::set<int> liveOutIDs;

  std::unordered_map<Value *, std::set<Value *>> prodConsumers;

  bool hasExitBlockEnv;
  Type *exitBlockType;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_ENVIRONMENT_LOOPENVIRONMENT_H_
