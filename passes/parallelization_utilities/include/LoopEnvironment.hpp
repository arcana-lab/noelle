#pragma once

#include <unordered_map>
#include <vector>
#include <set>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "PDG.hpp"

namespace llvm {

  class LoopEnvironment {
    public:
      LoopEnvironment (PDG *loopDG, SmallVector<BasicBlock *, 10> &exitBlocks);

      iterator_range<std::vector<Value *>::iterator> getProducers() { 
        return make_range(envProducers.begin(), envProducers.end());
      }

      iterator_range<std::set<int>::iterator> getPreEnvIndices() { 
        return make_range(preLoopEnv.begin(), preLoopEnv.end());
      }

      iterator_range<std::set<int>::iterator> getPostEnvIndices() { 
        return make_range(postLoopEnv.begin(), postLoopEnv.end());
      }

      /*
       * One per external dependent + one to track exit block
       */
      int envSize (void) const ;

      int indexOfExitBlock (void) const ;

      Type *typeOfEnv (int index) const ;

      bool isPreLoopEnv (Value *val);

      inline Value *producerAt (int ind) { return envProducers[ind]; }

      std::set<Value *> &consumersOf (Value *prod);

      /*
       * DEPRECATED(angelo): use of this API suggests poor environment 
       * algorithm, as all producers should just be iterated over for
       * any arbitrary operation
       */
      int indexOfProducer (Value *producer);

      bool isProducer (Value *producer) const ;

    private:
      void addPreLoopProducer (Value *producer);
      void addPostLoopProducer (Value *producer);
      void addProducer (Value *producer, bool preLoop);

      std::vector<Value *> envProducers;
      std::unordered_map<Value *, int> producerIndexMap;

      std::set<int> preLoopEnv;
      std::set<int> postLoopEnv;

      std::unordered_map<Value *, std::set<Value *>> prodConsumers;

      bool hasExitBlockEnv;
      Type *exitBlockType;
  };

}
