#pragma once

#include "TaskExecution.hpp"
#include "SCCDAGAttrs.hpp"

using namespace std;

namespace llvm {

  struct HELIXTask : TaskExecution {

    /*
     * Task arguments
     */
    Value *coreArg, *numCoresArg;

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
