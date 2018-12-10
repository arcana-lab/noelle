#pragma once

#include "HELIXTask.hpp"

void llvm::HELIXTask::extractFuncArgs (void) {
  auto argIter = this->F->arg_begin();
  this->envArg = (Value *) &*(argIter++);
  this->coreArg = (Value *) &*(argIter++); 
  this->numCoresArg = (Value *) &*(argIter++);
  this->instanceIndexV = coreArg;

  return ;
}
