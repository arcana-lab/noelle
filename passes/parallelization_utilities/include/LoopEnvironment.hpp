#pragma once

#include <unordered_map>
#include <vector>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

namespace llvm {

  class LoopEnvironment {
    public:
      std::vector<Value *> envProducers;
      unordered_map<Value *, int> producerIndexMap;
      unordered_map<Value *, set<Value *>> prodConsumers;
      std::set<int> preLoopEnv;
      std::set<int> postLoopEnv;

      Type *exitBlockType;

      /*
       * One per external dependent + one to track exit block
       */
      int envSize () { return envProducers.size() + 1; }
      int indexOfExitBlock () { return envProducers.size(); }

      Type *typeOfEnv (int index)
      {
          if (index < envProducers.size()) return envProducers[index]->getType();
          return exitBlockType;
      }

      void addProducer (Value *producer, bool preLoop)
      {
          auto envIndex = envProducers.size();
          envProducers.push_back(producer);
          producerIndexMap[producer] = envIndex;
          if (preLoop) preLoopEnv.insert(envIndex);
          else postLoopEnv.insert(envIndex);
      }
      void addPreLoopProducer (Value *producer) { addProducer(producer, true); }
      void addPostLoopProducer (Value *producer) { addProducer(producer, false); }

      bool isPreLoopEnv(Value *val)
      {
          return producerIndexMap.find(val) != producerIndexMap.end() && preLoopEnv.find(producerIndexMap[val]) != preLoopEnv.end();
      }
  };

}
