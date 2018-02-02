#pragma once;

#include "PDGBase.hpp"

using namespace llvm;

namespace llvm {
  class PDGNodePrintState {
   public:
    PDGNodePrintState(PDGNodeBase<Instruction> *n) { node = n; hidden = false; description = ""; }

    bool isNodeHidden() {
      return hidden;
    }
    
    std::string getNodeDescription() {
      return description;
    }

   private:
    PDGNodeBase<Instruction> *node;
    bool hidden;
    std::string description;
  };
}