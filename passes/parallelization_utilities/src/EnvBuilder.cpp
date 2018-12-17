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

EnvUserBuilder::EnvUserBuilder (LoopEnvironment &le)
  : LE{le}, envIndexToPtr{}, liveInInds{}, liveOutInds{} {
  envIndexToPtr.clear();
  liveInInds.clear();
  liveOutInds.clear();
}

EnvUserBuilder::~EnvUserBuilder () {
}

void EnvUserBuilder::createEnvPtr (
  IRBuilder<> builder,
  int envIndex
) {
  if (!this->envArray) {
    errs() << "A reference to the environment array has not been set for this user!\n";
    abort();
  }

  auto int64 = IntegerType::get(builder.getContext(), 64);
  auto envIndV = cast<Value>(ConstantInt::get(int64, envIndex));

  auto envGEP = builder.CreateInBoundsGEP(
    this->envArray,
    ArrayRef<Value*>({
      cast<Value>(ConstantInt::get(int64, 0)),
      envIndV
    })
  );
  auto envPtr = builder.CreateBitCast(
    builder.CreateLoad(envGEP),
    PointerType::getUnqual(this->LE.typeOfEnv(envIndex))
  );

  this->envIndexToPtr[envIndex] = cast<Instruction>(envPtr);
}

void EnvUserBuilder::createReducableEnvPtr (
  IRBuilder<> builder,
  int envIndex,
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
  auto envIndV = cast<Value>(ConstantInt::get(int64, envIndex));

  auto envReduceGEP = builder.CreateInBoundsGEP(
    this->envArray,
    ArrayRef<Value*>({
      zeroV,
      envIndV
    })
  );
  auto envReducePtr = builder.CreateBitCast(
    builder.CreateLoad(envReduceGEP),
    PointerType::getUnqual(ArrayType::get(ptrTy_int8, reducerCount))
  );

  auto envGEP = builder.CreateInBoundsGEP(
    envReducePtr,
    ArrayRef<Value*>({
      zeroV,
      reducerIndV,
    })
  );
  auto envPtr = builder.CreateBitCast(
    builder.CreateLoad(envGEP),
    PointerType::getUnqual(this->LE.typeOfEnv(envIndex))
  );

  this->envIndexToPtr[envIndex] = cast<Instruction>(envPtr);
}

EnvBuilder::EnvBuilder (LoopEnvironment &le, LLVMContext &CXT)
  : LE{le}, envIndexToVar{}, envIndexToReducableVar{},
    envUsers{}, numReducers{-1} {
  auto int8 = IntegerType::get(CXT, 8);
  auto ptrTy_int8 = PointerType::getUnqual(int8);
  this->envArrayType = ArrayType::get(ptrTy_int8, this->LE.envSize());
  envIndexToVar.clear();
  envIndexToReducableVar.clear();
  envUsers.clear();
  envArray = envArrayInt8Ptr = nullptr;
}

EnvBuilder::~EnvBuilder () {
  for (auto user : envUsers) delete user;
}

void EnvBuilder::createEnvUsers (int numUsers) {
  for (int i = 0; i < numUsers; ++i) {
    this->envUsers.push_back(new EnvUserBuilder(this->LE));
  }
}

void EnvBuilder::createEnvVariables (
  std::set<int> &singleVarIndices,
  std::set<int> &reducableVarIndices,
  int reducerCount
) {
  numReducers = reducerCount;
  for (auto envIndex : singleVarIndices) {
    envIndexToVar[envIndex] = nullptr;
  }
  for (auto envIndex : reducableVarIndices) {
    envIndexToReducableVar[envIndex] = std::vector<Value *>();
  }
}

void EnvBuilder::generateEnvArray (IRBuilder<> builder) {
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

  /*
   * Check the variables created.
   */
  auto variablesCreated = envIndexToVar.size() + envIndexToReducableVar.size();
  if (variablesCreated != this->LE.envSize()) {
    errs() << "EnvBuilder: ERROR = Not all environment variables have been created.\n" ;
    errs() << "\tEnvironment varibles = " << LE.envSize() << "\n";
    errs() << "\tVariables created = " << variablesCreated << "\n";
    errs() << "\tSee the EnvBuilder API call createEnvVariables\n";
    abort();
  }

  auto int8 = IntegerType::get(builder.getContext(), 8);
  auto ptrTy_int8 = PointerType::getUnqual(int8);
  auto int64 = IntegerType::get(builder.getContext(), 64);
  auto zeroV = cast<Value>(ConstantInt::get(int64, 0));
  auto storeEnvAllocaInArray = [&](Value *arr, int envIndex, AllocaInst *alloca) -> void {
    auto indValue = cast<Value>(ConstantInt::get(int64, envIndex));
    auto envPtr = builder.CreateInBoundsGEP(arr, ArrayRef<Value*>({ zeroV, indValue }));
    auto depCast = builder.CreateBitCast(envPtr, PointerType::getUnqual(alloca->getType()));
    auto store = builder.CreateStore(alloca, depCast);
  };

  std::set<int> singleVars;
  for (auto indexVarPair : envIndexToVar) {
    singleVars.insert(indexVarPair.first);
  }
  for (auto envIndex : singleVars) {
    Type *envType = this->LE.typeOfEnv(envIndex);
    auto varAlloca = builder.CreateAlloca(envType);
    envIndexToVar[envIndex] = varAlloca;

    storeEnvAllocaInArray(this->envArray, envIndex, varAlloca);
  }

  std::set<int> reducableVars;
  for (auto indexVarPair : envIndexToReducableVar) {
    reducableVars.insert(indexVarPair.first);
  }
  for (auto envIndex : reducableVars) {
    auto reduceArrType = ArrayType::get(ptrTy_int8, numReducers);
    auto reduceArrAlloca = builder.CreateAlloca(reduceArrType);

    storeEnvAllocaInArray(this->envArray, envIndex, reduceArrAlloca);

    /*
     * Create environment variable's array, one slot per user
     */
    Type *envType = this->LE.typeOfEnv(envIndex);
    for (auto i = 0; i < numReducers; ++i) {
      auto varAlloca = builder.CreateAlloca(envType);
      envIndexToReducableVar[envIndex].push_back(varAlloca);

      storeEnvAllocaInArray(reduceArrAlloca, i, varAlloca);
    }
  }
}

void EnvBuilder::reduceLiveOutVariables (
  IRBuilder<> builder,
  std::unordered_map<int, int> &reducableBinaryOps,
  std::unordered_map<int, Value *> &initialValues
) {
  for (auto envIndexInitValue : initialValues) {
    auto envIndex = envIndexInitValue.first;
    auto initialValue = envIndexInitValue.second;
    auto binOp = (Instruction::BinaryOps)reducableBinaryOps[envIndex];

    /*
     * Reduce environment variable's array
     */
    Value *accumVal = builder.CreateLoad(this->getReducableEnvVar(envIndex, 0));
    for (auto i = 1; i < numReducers; ++i) {
      auto envVar = builder.CreateLoad(this->getReducableEnvVar(envIndex, i));
      accumVal = builder.CreateBinOp(binOp, accumVal, envVar);
    }

    accumVal = builder.CreateBinOp(binOp, accumVal, initialValue);
    envIndexToVar[envIndex] = accumVal;
  }
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
