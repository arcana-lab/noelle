/*
 * Copyright 2019 - 2021  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "Utils.hpp"

namespace llvm::noelle {

bool Utils::isActualCode (Instruction *inst){
  auto callInst = dyn_cast<CallInst>(inst);
  if (callInst == nullptr){
    return true;
  }
  if (callInst->isLifetimeStartOrEnd()){
    return false;
  }

  return true;
}

bool Utils::isAllocator (CallInst *callInst){

  /*
   * Check the instruction.
   */
  if (callInst == nullptr){
    return false;
  }

  /*
   * Fetch the callee.
   */
  auto callee = callInst->getCalledFunction();
  if (callee == nullptr){
    return false;
  }

  /*
   * Check if it is a call to a library.
   */
  if (!callee->empty()){
    return false;
  }

  /*
   * Check if it is a call to a known library function.
   */
  auto calleeName = callee->getName();
  if (  false
        || (calleeName == "malloc")
        || (calleeName == "calloc")
        || (calleeName == "realloc")
    ){
    return true;
  }

  return false;
}

bool Utils::isReallocator (CallInst *callInst){

  /*
   * Check the instruction.
   */
  if (callInst == nullptr){
    return false;
  }

  /*
   * Fetch the callee.
   */
  auto callee = callInst->getCalledFunction();
  if (callee == nullptr){
    return false;
  }

  /*
   * Check if it is a call to a library.
   */
  if (!callee->empty()){
    return false;
  }

  /*
   * Check if it is a call to a known library function.
   */
  auto calleeName = callee->getName();
  if (  false
        || (calleeName == "realloc")
    ){
    return true;
  }

  return false;
}

bool Utils::isDeallocator (CallInst *callInst){

  /*
   * Check the instruction.
   */
  if (callInst == nullptr){
    return false;
  }

  /*
   * Fetch the callee.
   */
  auto callee = callInst->getCalledFunction();
  if (callee == nullptr){
    return false;
  }

  /*
   * Check if it is a call to a library.
   */
  if (!callee->empty()){
    return false;
  }

  /*
   * Check if it is a call to a known library function.
   */
  auto calleeName = callee->getName();
  if (  false
        || (calleeName == "free")
    ){
    return true;
  }

  return false;
}

Value * Utils::getAllocatedObject (CallInst *call){
  if (!Utils::isAllocator(call)){
    return nullptr;
  }
  auto callee = call->getCalledFunction();
  if (callee == nullptr){
    return nullptr;
  }
  auto calleeName = callee->getName();
  if (  false
        || (calleeName == "malloc")
        || (calleeName == "calloc")
        || (calleeName == "realloc")
     ){
    return call;
  }

  /*
   * TODO: complete this function with other allocators (e.g., posix_memalign)
   */
  abort();
}

Value * Utils::getFreedObject (CallInst *call){
  if (!Utils::isDeallocator(call)){
      return nullptr;
  }
  auto callee = call->getCalledFunction();
  if (callee == nullptr){
    return nullptr;
  }
  auto calleeName = callee->getName();
  if (  false
        || (calleeName == "free")
     ){
    return call->getArgOperand(0);
  }

  /*
   * TODO: complete this function with other deallocators
   */
  abort();
}

}
