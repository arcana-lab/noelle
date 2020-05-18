/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "EnvBuilder.hpp"
#include "Architecture.hpp"

using namespace llvm ;

EnvUserBuilder::EnvUserBuilder ()
  : envIndexToPtr{}, liveInInds{}, liveOutInds{} {
  envIndexToPtr.clear();
  liveInInds.clear();
  liveOutInds.clear();
}

EnvUserBuilder::~EnvUserBuilder () {
}

void EnvUserBuilder::createEnvPtr (
  IRBuilder<> builder,
  int envIndex,
  Type *type
) {
  if (!this->envArray) {
    errs() << "A reference to the environment array has not been set for this user!\n";
    abort();
  }

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

void EnvUserBuilder::createReducableEnvPtr (
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

EnvBuilder::EnvBuilder (LLVMContext &cxt)
  : CXT{cxt}, envTypes{}, envUsers{},
    envIndexToVar{}, envIndexToReducableVar{}, envIndexToVectorOfReducableVar{},
    numReducers{-1}, envSize{-1} {
  envIndexToVar.clear();
  envIndexToReducableVar.clear();
  envIndexToVectorOfReducableVar.clear();
  envUsers.clear();
  envArrayType = nullptr;
  envArray = envArrayInt8Ptr = nullptr;

  return ;
}

EnvBuilder::~EnvBuilder () {
  for (auto user : envUsers) delete user;
}

void EnvBuilder::createEnvUsers (int numUsers) {
  for (int i = 0; i < numUsers; ++i) {
    this->envUsers.push_back(new EnvUserBuilder());
  }
}

// TODO: Adjust users of createEnvVariables to pass the Type map
void EnvBuilder::createEnvVariables (
  std::vector<Type *> &varTypes,
  std::set<int> &singleVarIndices,
  std::set<int> &reducableVarIndices,
  int reducerCount
) {
  assert(envSize == -1 && "Environment variables must be fully determined at once\n");
  this->envSize = singleVarIndices.size() + reducableVarIndices.size();

  assert(this->envSize == varTypes.size()
    && "Environment variables must either be singular or reducible\n");
  this->envTypes = std::vector<Type *>(varTypes.begin(), varTypes.end());

  /*
   * Compute how many values can fit in a cache line.
   */
  auto valuesInCacheLine = Architecture::getCacheLineBytes() / sizeof(int64_t);

  auto int64 = IntegerType::get(this->CXT, 64);
  this->envArrayType = ArrayType::get(int64, this->envSize * valuesInCacheLine);

  numReducers = reducerCount;
  for (auto envIndex : singleVarIndices) {
    envIndexToVar[envIndex] = nullptr;
  }
  for (auto envIndex : reducableVarIndices) {
    envIndexToReducableVar[envIndex] = std::vector<Value *>();
  }
}

void EnvBuilder::generateEnvArray (IRBuilder<> builder) {
  if(envSize == -1) {
    errs() << "Environment array variables must be specified!\n"
      << "\tSee the EnvBuilder API call createEnvVariables\n";
    abort();
  }

  auto int8 = IntegerType::get(builder.getContext(), 8);
  auto ptrTy_int8 = PointerType::getUnqual(int8);
  this->envArray = builder.CreateAlloca(this->envArrayType);
  this->envArrayInt8Ptr = cast<Value>(builder.CreateBitCast(this->envArray, ptrTy_int8));
}

void EnvBuilder::generateEnvVariables (IRBuilder<> builder) {

  /*
   * Check the environment array.
   */
  if (!this->envArray) {
    errs() << "An environment array has not been generated!\n"
      << "\tSee the EnvBuilder API call generateEnvArray\n";
    abort();
  }

  auto int8 = IntegerType::get(builder.getContext(), 8);
  auto ptrTy_int8 = PointerType::getUnqual(int8);
  auto int64 = IntegerType::get(builder.getContext(), 64);
  auto zeroV = cast<Value>(ConstantInt::get(int64, 0));
  auto fetchCastedEnvPtr = [&](Value *arr, int envIndex, Type *ptrType) -> Value * {

    /*
     * Compute the offset of the variable with index "envIndex" that is stored inside the environment.
     *
     * Compute how many values can fit in a cache line.
     */
    auto valuesInCacheLine = Architecture::getCacheLineBytes() / sizeof(int64_t);
    auto indValue = cast<Value>(ConstantInt::get(int64, envIndex * valuesInCacheLine));

    /*
     * Compute the address of the variable with index "envIndex".
     */
    auto envPtr = builder.CreateInBoundsGEP(arr, ArrayRef<Value*>({ zeroV, indValue }));

    /*
     * Cast the pointer to the proper data type.
     */
    auto cast = builder.CreateBitCast(envPtr, ptrType);

    return cast;
  };

  /*
   * Compute and cache the pointer of each variable that cannot be reduced and that are stored inside the environment.
   *
   * NOTE: Manipulation of the map cannot be done while iterating it
   */
  std::set<int> singleIndices;
  for (auto indexVarPair : envIndexToVar){
    singleIndices.insert(indexVarPair.first);
  }
  for (auto envIndex : singleIndices) {
    auto ptrType = PointerType::getUnqual(envTypes[envIndex]);
    envIndexToVar[envIndex] = fetchCastedEnvPtr(this->envArray, envIndex, ptrType);
  }

  /*
   * Vectorize reducable variables.
   * Moreover, compute and cache the pointer of each reducable variable that are stored inside the environment.
   *
   * NOTE: No manipulation and iteration at the same time
   */
  std::set<int> reducableIndices;
  for (auto indexVarPair : envIndexToReducableVar){
    reducableIndices.insert(indexVarPair.first);
  }
  for (auto envIndex : reducableIndices) {

    /*
     * Fetch the type of the current reducable variable.
     */
    auto varType = envTypes[envIndex];
    auto ptrType = PointerType::getUnqual(varType);

    /*
     * Define the type of the vectorized form of the reducable variable.
     */
    auto valuesInCacheLine = Architecture::getCacheLineBytes() / sizeof(int64_t);
    auto reduceArrType = ArrayType::get(int64, numReducers * valuesInCacheLine);

    /*
     * Allocate the vectorized form of the reducable variable on the stack.
     */
    auto reduceArrAlloca = builder.CreateAlloca(reduceArrType);
    envIndexToVectorOfReducableVar[envIndex] = reduceArrAlloca;

    /*
     * Store the pointer of the vector of the reducable variable inside the environment.
     */
    auto reduceArrPtrType = PointerType::getUnqual(reduceArrAlloca->getType());
    auto envPtr = fetchCastedEnvPtr(this->envArray, envIndex, reduceArrPtrType);
    builder.CreateStore(reduceArrAlloca, envPtr);

    /*
     * Compute and cache the pointer of each element of the vectorized variable.
     */
    for (auto i = 0; i < numReducers; ++i) {
      auto reducePtr = fetchCastedEnvPtr(reduceArrAlloca, i, ptrType);
      envIndexToReducableVar[envIndex].push_back(reducePtr);
    }
  }

  return ;
}

BasicBlock * EnvBuilder::reduceLiveOutVariables (
  BasicBlock *bb,
  IRBuilder<> builder,
  std::unordered_map<int, int> &reducableBinaryOps,
  std::unordered_map<int, Value *> &initialValues,
  Value *numberOfThreadsExecuted
) {

  /*
   * Check if there are any live-out variable that needs to be reduced.
   */
  if (initialValues.size() == 0){
    return bb;
  }

  /*
   * Fetch the function that "bb" belongs to.
   */
  auto f = bb->getParent();

  /*
   * Create a new basic block that will include the loop body.
   */
  auto loopBodyBB = BasicBlock::Create(this->CXT, "ReductionLoopBody", f);

  /*
   * Create a new basic block that will include the code after the reduction loop.
   */
  auto afterReductionBB = BasicBlock::Create(this->CXT, "AfterReduction", f, loopBodyBB);

  /*
   * Change the successor of "bb" to be "loopBodyBB".
   */
  auto bbTerminator = bb->getTerminator();
  if (bbTerminator != nullptr){
    bbTerminator->eraseFromParent();
  }
  IRBuilder<> bbBuilder{bb};
  bbBuilder.CreateBr(loopBodyBB);

  /*
   * Add the PHI node about the induction variable of the reduction loop.
   */
  IRBuilder<> loopBodyBuilder{loopBodyBB};
  auto int32Type = IntegerType::get(builder.getContext(), 32);
  auto IVReductionLoop = loopBodyBuilder.CreatePHI(int32Type, 2);
  auto constantZero = ConstantInt::get(int32Type, 0);
  IVReductionLoop->addIncoming(constantZero, bb);

  /*
   * Add the PHI nodes about the current accumulated value
   */
  std::vector<PHINode *> phiNodes;
  auto count = 0;
  for (auto envIndexInitValue : initialValues) {
    auto envIndex = envIndexInitValue.first;
    auto initialValue = envIndexInitValue.second;

    /*
     * Create a PHI node for the current reduced variable.
     */
    auto variableType = envTypes[envIndex];
    auto phiNode = loopBodyBuilder.CreatePHI(variableType, 2);

    /*
     * Add the value in case we just started accumulating.
     */
    phiNode->addIncoming(initialValue, bb);

    /*
     * Keep track of the PHI node just created.
     */
    phiNodes.push_back(phiNode);
  }

  /*
   * Compute how many values can fit in a cache line.
   */
  auto valuesInCacheLine = Architecture::getCacheLineBytes() / sizeof(int64_t);

  /*
   * Load the values stored in the private copies of the threads.
   */
  count = 0;
  std::vector<Value *> loadedValues;
  for (auto envIndexInitValue : initialValues) {
    auto envIndex = envIndexInitValue.first;

    /*
     * Compute the pointer of the private copy of the current thread.
     *
     * First, we compute the offset, which is "index" times 8 because environment values are 64 byte aligned.
     */
    auto eightValue = ConstantInt::get(int32Type, valuesInCacheLine);
    auto offsetValue = loopBodyBuilder.CreateMul(IVReductionLoop, eightValue);

    /*
     * Now, we compute the effective address.
     */
    auto baseAddressOfReducedVar = envIndexToVectorOfReducableVar[envIndex];
    auto zeroV = cast<Value>(ConstantInt::get(int32Type, 0));
    auto effectiveAddressOfReducedVar = loopBodyBuilder.CreateInBoundsGEP(baseAddressOfReducedVar, ArrayRef<Value*>({ zeroV, offsetValue}));

    /*
     * Finally, cast the effective address to the correct LLVM type.
     */
    auto varType = envTypes[envIndex];
    auto ptrType = PointerType::getUnqual(varType);
    auto effectiveAddressOfReducedVarProperlyCasted = loopBodyBuilder.CreateBitCast(effectiveAddressOfReducedVar, ptrType);

    /*
     * Load the next value that needs to be accumulated.
     */
    auto envVar = loopBodyBuilder.CreateLoad(effectiveAddressOfReducedVarProperlyCasted);
    loadedValues.push_back(envVar);
  }

  /*
   * Accumulate values to the appropriate accumulators.
   */
  count = 0;
  for (auto envIndexInitValue : initialValues) {
    auto envIndex = envIndexInitValue.first;

    /*
     * Fetch the information about the operation to perform to accumulate values.
     */
    auto binOp = (Instruction::BinaryOps)reducableBinaryOps[envIndex];

    /*
     * Fetch the accumulator, which is the PHI node related to the current reduced variable.
     */
    auto accumVal = phiNodes[count];

    /*
     * Accumulate values to the accumulator of the current reduced variable.
     */
    auto privateCurrentCopy = loadedValues[count];
    auto newAccumulatorValue = loopBodyBuilder.CreateBinOp(binOp, accumVal, privateCurrentCopy);

    /*
     * Keep track of the new accumulator value.
     */
    envIndexToAccumulatedReducableVar[envIndex] = newAccumulatorValue;

    count++;
  }

  /*
   * Fix the PHI nodes of the accumulators.
   */
  count = 0;
  for (auto envIndexInitValue : initialValues) {
    auto envIndex = envIndexInitValue.first;

    /*
     * Fetch the PHI node of the accumulator of the current reduced variable.
     */
    auto phiNode = phiNodes[count];

    /*
     * Fetch the value computed by the previous iteration.
     */
    auto previousIterationAccumulatorValue = envIndexToAccumulatedReducableVar[envIndex];

    /*
     * Add the value related to the previous iteration of the reduction loop.
     */
    phiNode->addIncoming(previousIterationAccumulatorValue, loopBodyBB);

    count++;
  }

  /*
   * Update the induction variable for the reduction loop.
   */
  auto constantOne = ConstantInt::get(int32Type, 1);
  auto updatedIVReductionLoop = loopBodyBuilder.CreateAdd(IVReductionLoop, constantOne);
  IVReductionLoop->addIncoming(updatedIVReductionLoop, loopBodyBB);

  /*
   * Compute the condition to jump back to the reduction loop body.
   */
  auto continueToReduceVariables = loopBodyBuilder.CreateICmpSLT(updatedIVReductionLoop, numberOfThreadsExecuted);

  /*
   * Add the successors of "loopBodyBB" to be either back to "loopBodyBB" or "afterReductionBB".
   */
  loopBodyBuilder.CreateCondBr(continueToReduceVariables, loopBodyBB, afterReductionBB);

  return afterReductionBB;
}

Value *EnvBuilder::getEnvArrayInt8Ptr () {
  assert(envArrayInt8Ptr);
  return envArrayInt8Ptr;
}

Value *EnvBuilder::getEnvArray () {
  assert(envArray);
  return envArray;
}

Value *EnvBuilder::getEnvVar (int ind) {
  auto iter = envIndexToVar.find(ind);
  assert(iter != envIndexToVar.end());
  return (*iter).second;
}

Value *EnvBuilder::getAccumulatedReducableEnvVar (int ind) {
  auto iter = envIndexToAccumulatedReducableVar.find(ind);
  assert(iter != envIndexToAccumulatedReducableVar.end());
  return (*iter).second;
}

Value *EnvBuilder::getReducableEnvVar (int ind, int reducerInd) {
  auto iter = envIndexToReducableVar.find(ind);
  assert(iter != envIndexToReducableVar.end());
  return (*iter).second[reducerInd];
}

bool EnvBuilder::isReduced (int ind) {
  auto isSingle = envIndexToVar.find(ind) != envIndexToVar.end();
  auto isReduce = envIndexToReducableVar.find(ind) != envIndexToReducableVar.end();
  assert(isSingle || isReduce);
  return isReduce;
}
