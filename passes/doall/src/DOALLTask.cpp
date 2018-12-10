#include "DOALLTask.hpp"

void llvm::DOALLTask::extractFuncArgs (void) {
  auto argIter = this->F->arg_begin();
  this->envArg = (Value *) &*(argIter++);
  this->coreArg = (Value *) &*(argIter++); 
  this->numCoresArg = (Value *) &*(argIter++);
  this->chunkSizeArg = (Value *) &*(argIter++);
  this->instanceIndexV = coreArg;

  return ;
}
