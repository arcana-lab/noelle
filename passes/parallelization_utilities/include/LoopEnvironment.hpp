#pragma once

#include <unordered_map>
#include <vector>
#include <set>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

namespace llvm {

  class LoopEnvironment {
    public:
      std::vector<Value *> envProducers;
      std::unordered_map<Value *, int> producerIndexMap;
      std::unordered_map<Value *, std::set<Value *>> prodConsumers;
      std::set<int> preLoopEnv;
      std::set<int> postLoopEnv;
      Type *exitBlockType;
      ArrayType *envArrayType;
      Value *envArray;

      /*
       * One per external dependent + one to track exit block
       */
      int envSize (void);

      int indexOfExitBlock (void);

      Type *typeOfEnv (int index);

      void addProducer (Value *producer, bool preLoop);

      void addPreLoopProducer (Value *producer);

      void addPostLoopProducer (Value *producer);

      bool isPreLoopEnv(Value *val);
  };

}
