#pragma once

#include "TaskExecution.hpp"

using namespace std;

namespace llvm {

  struct DOALLTaskExecution : TaskExecution {

    /*
     * Inner loop header/latch
     */
    BasicBlock *outermostLoopHeader, *outermostLoopLatch;

    /*
     * Chunking function specific arguments
     */
    Value *coreArg, *numCoresArg, *chunkSizeArg;

    /*
     * Clone of original IV loop, new outer loop
     */
    SCCAttrs *originalIVAttrs;
    SimpleIVInfo clonedIVInfo;
    PHINode *originalIVClone;
    PHINode *outermostLoopIV;

    void extractFuncArgs () override {
      auto argIter = this->F->arg_begin();
      this->envArg = (Value *) &*(argIter++);
      this->coreArg = (Value *) &*(argIter++); 
      this->numCoresArg = (Value *) &*(argIter++);
      this->chunkSizeArg = (Value *) &*(argIter++);
      this->instanceIndexV = coreArg;
    }
  };
}
