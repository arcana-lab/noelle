#include "EnvBuilder.hpp"

using namespace llvm ;

EnvUserBuilder::EnvUserBuilder (LoopEnvironment &le) : LE{le} {} 

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

EnvBuilder::EnvBuilder (LoopEnvironment &le, LLVMContext &CXT) : LE{le} {
  auto int8 = IntegerType::get(CXT, 8);
  auto ptrTy_int8 = PointerType::getUnqual(int8);
  this->envArrayType = ArrayType::get(ptrTy_int8, this->LE.envSize());
}

void EnvBuilder::createEnvArray (IRBuilder<> builder) {
  auto int8 = IntegerType::get(builder.getContext(), 8);
  auto ptrTy_int8 = PointerType::getUnqual(int8);
  this->envArray = builder.CreateAlloca(this->envArrayType);
  this->envArrayInt8Ptr = cast<Value>(builder.CreateBitCast(this->envArray, ptrTy_int8));
}

void EnvBuilder::createEnvUsers (int numUsers) {
  for (int i = 0; i < numUsers; ++i) {
    this->envUsers.push_back(new EnvUserBuilder(this->LE));
  }
}

void EnvBuilder::allocateEnvVariables (
  IRBuilder<> builder,
  std::set<int> &singleVarIndices,
  std::set<int> &reducableVarIndices,
  int reducerCount
) {
  if (!this->envArray) {
    errs() << "An environment array has not been created!\n";
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

  for (auto envIndex : singleVarIndices) {
    Type *envType = this->LE.typeOfEnv(envIndex);
    auto varAlloca = builder.CreateAlloca(envType);
    envIndexToVar[envIndex] = varAlloca;

    storeEnvAllocaInArray(this->envArray, envIndex, varAlloca);
  }

  for (auto envIndex : reducableVarIndices) {
    auto reduceArrType = ArrayType::get(ptrTy_int8, reducerCount);
    auto reduceArrAlloca = builder.CreateAlloca(reduceArrType);

    storeEnvAllocaInArray(this->envArray, envIndex, reduceArrAlloca);

    /*
     * Create environment variable's array, one slot per user
     */
    Type *envType = this->LE.typeOfEnv(envIndex);
    for (auto i = 0; i < reducerCount; ++i) {
      auto varAlloca = builder.CreateAlloca(envType);
      envIndexToReducableVar[envIndex].push_back(varAlloca);

      storeEnvAllocaInArray(reduceArrAlloca, i, varAlloca);
    }
  }
}

void EnvBuilder::reduceLiveOutVariables (
  IRBuilder<> builder,
  std::unordered_map<int, int> &reducableBinaryOps,
  std::unordered_map<int, Value *> &initialValues,
  int reducerCount
) {
  for (auto envIndexInitValue : initialValues) {
    auto envIndex = envIndexInitValue.first;
    auto initialValue = envIndexInitValue.second;
    auto binOp = (Instruction::BinaryOps)reducableBinaryOps[envIndex];

    /*
     * Reduce environment variable's array
     */
    Value *accumVal = builder.CreateLoad(this->getReducableEnvVar(envIndex, 0));
    for (auto i = 1; i < reducerCount; ++i) {
      auto envVar = builder.CreateLoad(this->getReducableEnvVar(envIndex, i));
      accumVal = builder.CreateBinOp(binOp, accumVal, envVar);
    }

    accumVal = builder.CreateBinOp(binOp, accumVal, initialValue);
    envIndexToVar[envIndex] = accumVal;
  }
}
