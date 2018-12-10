#pragma once

#include "TaskExecution.hpp"
#include "SCCDAGAttrs.hpp"

namespace llvm {

  struct DOALLTask : TaskExecution {

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

    void extractFuncArgs () override ;
  };
}
