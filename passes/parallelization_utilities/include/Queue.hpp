#pragma once

#include <vector>

namespace llvm {

  class Queue {
    public:
      unordered_map<int, int> queueSizeToIndex;
      std::vector<Type *> queueElementTypes;
      std::vector<Function *> queuePushes;
      std::vector<Function *> queuePops;
      std::vector<Type *> queueTypes;
  };

}
