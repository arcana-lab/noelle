/*
 * Copyright 2021 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/Architecture.hpp"
#include "noelle/core/LoopEnvironmentUser.hpp"

namespace llvm::noelle {

LoopEnvironmentUser::LoopEnvironmentUser ()
  :   envIndexToPtr{}
    , liveInInds{}
    , liveOutInds{} 
    {
  envIndexToPtr.clear();
  liveInInds.clear();
  liveOutInds.clear();

  return ;
}

void LoopEnvironmentUser::createEnvPtr (
  IRBuilder<> builder,
  int envIndex,
  Type *type
) {

  /*
   * Check the inputs.
   */
  if (!this->envArray) {
    errs() << "A reference to the environment array has not been set for this user!\n";
    abort();
  }

  /*
   * Create the zero integer constant.
   */
  auto int64 = IntegerType::get(builder.getContext(), 64);
  auto zeroV = cast<Value>(ConstantInt::get(int64, 0));

  /*
   * Compute how many values can fit in a cache line.
   */
  auto valuesInCacheLine = Architecture::getCacheLineBytes() / sizeof(int64_t);

  auto envIndV = cast<Value>(ConstantInt::get(int64, envIndex * valuesInCacheLine));

  auto envGEP = builder.CreateInBoundsGEP(
    this->envArray,
    ArrayRef<Value*>({ zeroV, envIndV })
  );
  auto envPtr = builder.CreateBitCast(envGEP, PointerType::getUnqual(type));

  this->envIndexToPtr[envIndex] = cast<Instruction>(envPtr);
}

void LoopEnvironmentUser::createReducableEnvPtr (
  IRBuilder<> builder,
  int envIndex,
  Type *type,
  int reducerCount,
  Value *reducerIndV
) {
  if (!this->envArray) {
    errs() << "A reference to the environment array has not been set for this user!\n";
    abort();
  }

  /*
   * Compute how many values can fit in a cache line.
   */
  auto valuesInCacheLine = Architecture::getCacheLineBytes() / sizeof(int64_t);

  auto int8 = IntegerType::get(builder.getContext(), 8);
  auto ptrTy_int8 = PointerType::getUnqual(int8);
  auto int64 = IntegerType::get(builder.getContext(), 64);
  auto zeroV = cast<Value>(ConstantInt::get(int64, 0));
  auto envIndV = cast<Value>(ConstantInt::get(int64, envIndex * valuesInCacheLine));

  auto envReduceGEP = builder.CreateInBoundsGEP(
    this->envArray,
    ArrayRef<Value*>({ zeroV, envIndV })
  );
  auto arrPtr = PointerType::getUnqual(ArrayType::get(int64, reducerCount * valuesInCacheLine));
  auto envReducePtr = builder.CreateBitCast(envReduceGEP, PointerType::getUnqual(arrPtr));

  auto reduceIndAlignedV = builder.CreateMul(reducerIndV, ConstantInt::get(int64, valuesInCacheLine));
  auto envGEP = builder.CreateInBoundsGEP(
    builder.CreateLoad(envReducePtr),
    ArrayRef<Value*>({ zeroV, reduceIndAlignedV })
  );
  auto envPtr = builder.CreateBitCast(envGEP, PointerType::getUnqual(type));

  this->envIndexToPtr[envIndex] = cast<Instruction>(envPtr);
}
    
void LoopEnvironmentUser::addLiveInIndex (uint32_t ind) { 
  liveInInds.insert(ind); 

  return ;
}

void LoopEnvironmentUser::addLiveOutIndex (uint32_t ind) { 
  liveOutInds.insert(ind); 

  return ;
}
    
Instruction * LoopEnvironmentUser::getEnvPtr (uint32_t ind) { 
  auto ptr = this->envIndexToPtr[ind]; 
  assert(ptr != nullptr);

  return ptr;
}

LoopEnvironmentUser::~LoopEnvironmentUser () {
  return ;
}

}
