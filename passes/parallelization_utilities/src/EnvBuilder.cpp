/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "EnvBuilder.hpp"

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
  auto envIndV = cast<Value>(ConstantInt::get(int64, envIndex * 8));

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

  auto int8 = IntegerType::get(builder.getContext(), 8);
  auto ptrTy_int8 = PointerType::getUnqual(int8);
  auto int64 = IntegerType::get(builder.getContext(), 64);
  auto zeroV = cast<Value>(ConstantInt::get(int64, 0));
  auto envIndV = cast<Value>(ConstantInt::get(int64, envIndex * 8));

  auto envReduceGEP = builder.CreateInBoundsGEP(
    this->envArray,
    ArrayRef<Value*>({ zeroV, envIndV })
  );
  auto arrPtr = PointerType::getUnqual(ArrayType::get(int64, reducerCount * 8));
  auto envReducePtr = builder.CreateBitCast(envReduceGEP, PointerType::getUnqual(arrPtr));

  auto reduceIndAlignedV = builder.CreateMul(reducerIndV, ConstantInt::get(int64, 8));
  auto envGEP = builder.CreateInBoundsGEP(
    builder.CreateLoad(envReducePtr),
    ArrayRef<Value*>({ zeroV, reduceIndAlignedV })
  );
  auto envPtr = builder.CreateBitCast(envGEP, PointerType::getUnqual(type));

  this->envIndexToPtr[envIndex] = cast<Instruction>(envPtr);
}

EnvBuilder::EnvBuilder (LLVMContext &cxt)
  : CXT{cxt}, envTypes{}, envUsers{},
    envIndexToVar{}, envIndexToReducableVar{},
    numReducers{-1}, envSize{-1} {
  envIndexToVar.clear();
  envIndexToReducableVar.clear();
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

  auto int64 = IntegerType::get(this->CXT, 64);
  this->envArrayType = ArrayType::get(int64, this->envSize * 8);

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
     * NOTE: environment values are 64 byte aligned.
     */
    auto indValue = cast<Value>(ConstantInt::get(int64, envIndex * 8));

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
    auto reduceArrType = ArrayType::get(int64, numReducers * 8);

    /*
     * Allocate the vectorized form of the reducable variable on the stack.
     */
    auto reduceArrAlloca = builder.CreateAlloca(reduceArrType);

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

void EnvBuilder::reduceLiveOutVariables (
  IRBuilder<> builder,
  std::unordered_map<int, int> &reducableBinaryOps,
  std::unordered_map<int, Value *> &initialValues,
  Value *numberOfThreadsExecuted
) {
  for (auto envIndexInitValue : initialValues) {
    auto envIndex = envIndexInitValue.first;
    auto initialValue = envIndexInitValue.second;
    auto binOp = (Instruction::BinaryOps)reducableBinaryOps[envIndex];

    /*
     * Load the accumulator of the current reduced variable.
     */
    Value *accumVal = builder.CreateLoad(this->getReducableEnvVar(envIndex, 0));

    /*
     * Accumulate values to the accumulator of the current reduced variable.
     */
    for (auto i = 1; i < numReducers; ++i) {

      /*
       * Load the next value that needs to be accumulated.
       */
      auto envVar = builder.CreateLoad(this->getReducableEnvVar(envIndex, i));

      /*
       * Reduce environment variable's array
       */
      accumVal = builder.CreateBinOp(binOp, accumVal, envVar);
    }

    accumVal = builder.CreateBinOp(binOp, accumVal, initialValue);
    envIndexToVar[envIndex] = accumVal;
  }

  return ;
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
