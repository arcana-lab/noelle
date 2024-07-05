/*
 * Copyright 2021 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "arcana/noelle/core/Architecture.hpp"
#include "arcana/noelle/core/LoopEnvironmentUser.hpp"

namespace arcana::noelle {

LoopEnvironmentUser::LoopEnvironmentUser(
    std::unordered_map<uint32_t, uint32_t> &envIDToIndex)
  : envIndexToPtr{},
    liveInIDs{},
    liveOutIDs{},
    envIDToIndex{ envIDToIndex } {
  envIndexToPtr.clear();
  liveInIDs.clear();
  liveOutIDs.clear();

  return;
}

void LoopEnvironmentUser::setEnvironmentArray(Value *envArr) {
  this->envArray = envArr;

  return;
}

Instruction *LoopEnvironmentUser::createEnvironmentVariablePointer(
    IRBuilder<> &builder,
    uint32_t envID,
    Type *type) {

  /*
   * Check the inputs.
   */
  if (!this->envArray) {
    errs()
        << "A reference to the environment array has not been set for this user!\n";
    abort();
  }

  /*
   * Mapping from envID to index
   */
  assert(this->envIDToIndex.find(envID) != this->envIDToIndex.end()
         && "The environment variable is not included in the user\n");
  auto envIndex = this->envIDToIndex.at(envID);

  /*
   * Create the zero integer constant.
   */
  auto int64 = IntegerType::get(builder.getContext(), 64);
  auto zeroV = cast<Value>(ConstantInt::get(int64, 0));

  /*
   * Compute how many values can fit in a cache line.
   */
  auto valuesInCacheLine = Architecture::getCacheLineBytes() / sizeof(int64_t);

  /*
   * Compute the offset of the environment variable.
   */
  auto envIndV =
      cast<Value>(ConstantInt::get(int64, envIndex * valuesInCacheLine));

  /*
   * Compute the address of the environment variable
   */
  auto envGEP =
      builder.CreateGEP(this->envArray->getType()->getStructElementType(0),
                        this->envArray,
                        ArrayRef<Value *>({ zeroV, envIndV }));
  auto envPtr = builder.CreateBitCast(envGEP, PointerType::getUnqual(type));

  /*
   * Cache the pointer of the environment variable.
   */
  auto ptrInst = cast<Instruction>(envPtr);
  this->envIndexToPtr[envIndex] = ptrInst;

  return ptrInst;
}

void LoopEnvironmentUser::createReducableEnvPtr(IRBuilder<> &builder,
                                                uint32_t envID,
                                                Type *type,
                                                uint32_t reducerCount,
                                                Value *reducerIndV) {
  if (!this->envArray) {
    errs()
        << "A reference to the environment array has not been set for this user!\n";
    abort();
  }

  /*
   * Mapping from envID to index
   */
  assert(this->envIDToIndex.find(envID) != this->envIDToIndex.end()
         && "The environment variable is not included in the user\n");
  auto envIndex = this->envIDToIndex.at(envID);

  /*
   * Compute how many values can fit in a cache line.
   */
  auto valuesInCacheLine = Architecture::getCacheLineBytes() / sizeof(int64_t);

  auto int64 = IntegerType::get(builder.getContext(), 64);
  auto zeroV = cast<Value>(ConstantInt::get(int64, 0));
  auto envIndV =
      cast<Value>(ConstantInt::get(int64, envIndex * valuesInCacheLine));

  auto envReduceGEP =
      builder.CreateGEP(this->envArray->getType()->getStructElementType(0),
                        this->envArray,
                        ArrayRef<Value *>({ zeroV, envIndV }));
  auto arrPtr = PointerType::getUnqual(
      ArrayType::get(int64, reducerCount * valuesInCacheLine));
  auto envReducePtr =
      builder.CreateBitCast(envReduceGEP, PointerType::getUnqual(arrPtr));

  auto reduceIndAlignedV =
      builder.CreateMul(reducerIndV,
                        ConstantInt::get(int64, valuesInCacheLine));
  auto newLoad =
      builder.CreateLoad(envReducePtr->getType()->getStructElementType(0),
                         envReducePtr);
  auto envGEP =
      builder.CreateGEP(newLoad->getType()->getStructElementType(0),
                        newLoad,
                        ArrayRef<Value *>({ zeroV, reduceIndAlignedV }));
  auto envPtr = builder.CreateBitCast(envGEP, PointerType::getUnqual(type));

  this->envIndexToPtr[envIndex] = cast<Instruction>(envPtr);
}

void LoopEnvironmentUser::addLiveIn(uint32_t id) {
  if (this->envIDToIndex.find(id) != this->envIDToIndex.end()) {
    liveInIDs.insert(id);
  }

  return;
}

void LoopEnvironmentUser::addLiveOut(uint32_t id) {
  if (this->envIDToIndex.find(id) != this->envIDToIndex.end()) {
    liveOutIDs.insert(id);
  }

  return;
}

Instruction *LoopEnvironmentUser::getEnvPtr(uint32_t id) {

  /*
   * Mapping from envID to index
   */
  assert(this->envIDToIndex.find(id) != this->envIDToIndex.end()
         && "The environment variable is not included in the user\n");
  auto ind = this->envIDToIndex.at(id);

  auto ptr = this->envIndexToPtr.at(ind);
  assert(ptr != nullptr);

  return ptr;
}

iterator_range<std::set<uint32_t>::iterator> LoopEnvironmentUser::
    getEnvIDsOfLiveInVars(void) {
  return make_range(liveInIDs.begin(), liveInIDs.end());
}

iterator_range<std::set<uint32_t>::iterator> LoopEnvironmentUser::
    getEnvIDsOfLiveOutVars(void) {
  return make_range(liveOutIDs.begin(), liveOutIDs.end());
}

LoopEnvironmentUser::~LoopEnvironmentUser() {
  return;
}

} // namespace arcana::noelle
