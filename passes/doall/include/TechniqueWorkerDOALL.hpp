#pragma once

#include "TechniqueWorker.hpp"

using namespace std;

namespace llvm {

  struct DOALLTechniqueWorker : TechniqueWorker {

    /*
     * Inner loop header/latch
     */
    BasicBlock *outerHeader, *outerLatch;

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
    PHINode *outerIV;

    void extractFuncArgs () {
      auto argIter = this->F->arg_begin();
      this->envArg = (Value *) &*(argIter++);
      this->coreArg = (Value *) &*(argIter++); 
      this->numCoresArg = (Value *) &*(argIter++);
      this->chunkSizeArg = (Value *) &*(argIter++);
      this->instanceIndexV = coreArg;
    }
  };
}
