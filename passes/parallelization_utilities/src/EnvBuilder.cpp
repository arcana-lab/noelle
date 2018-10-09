#include "EnvBuilder.hpp"

using namespace llvm ;

EnvUserBuilder::EnvUserBuilder (LoopEnvironment &le, Value *envArr)
    : LE{le}, envArray{envArr} {} 

EnvUserBuilder::createEnvPtrTo (IRBuilder<> builder, std::set<int> &userEnvIndices) {
  for (auto envIndex : userEnvIndices) {
    this->createEnvPtrTo(builder, envIndex);
  }
}

EnvUserBuilder::createEnvPtrTo (IRBuilder<> builder, int envIndex) {
  auto int64 = IntegerType::get(builder->getContext(), 64);
  auto envIndV = cast<Value>(ConstantInt::get(int64, envIndex));

  auto envGEP = builder.CreateInBoundsGEP(
    this->envArray,
    ArrayRef<Value*>({
      cast<Value>(ConstantInt::get(int64, 0)),
      envIndV
    })
  );
  auto envPtr = builder.CreateLoad(envGEP);
  this->envIndexToPtr[envIndex] = envPtr;
}

EnvBuilder::EnvBuilder (LoopEnvironment &le) : LE{le} {
  auto int8 = IntegerType::get(builder->getContext(), 8);
  auto ptrTy_int8 = PointerType::getUnqual(int8);
  this->envArrayType = ArrayType::get(ptrTy_int8, this->LE.envSize());
}

EnvBuilder::createEnvArray (IRBuilder<> builder) {
  auto int8 = IntegerType::get(builder->getContext(), 8);
  auto ptrTy_int8 = PointerType::getUnqual(int8);
  this->envArray = builder.CreateAlloca(this->envArrayType);
  this->envArrayInt8Ptr = cast<Value>(builder.CreateBitCast(this->envArray, ptrTy_int8));
  this->loopFunctionEnvBuilder = EnvUserBuilder(this->LE, this->envArray)
}

EnvBuilder::allocateEnvVariables (
  IRBuilder<> builder,
  std::set<int> &singleVarIndices,
  std::set<int> &reducableVarIndices,
  int reducerCount
) {
  auto int8 = IntegerType::get(builder->getContext(), 8);
  auto ptrTy_int8 = PointerType::getUnqual(int8);
  auto int64 = IntegerType::get(builder->getContext(), 64);
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

    storeEnvAllocaInArray(this->envArray, envIndex, varAlloca);

    /*
     * Insert pre-loop producers into the environment array
     */
    builder.CreateStore(this->LE.producerAt(envIndex), varAlloca);
  }

  for (auto envIndex : reducableVarIndices()) {
    auto reduceArrType = ArrayType::get(ptrTy_int8, reducerCount);
    auto reduceArrAlloca = entryBuilder.CreateAlloca(reduceArrType);

    storeEnvAllocaInArray(this->envArray, envIndex, reduceArrAlloca);

    Type *envType = this->LE.typeOfEnv(envIndex);
    for (auto i = 0; i < reducerCount; ++i) {
      auto varAlloca = builder.CreateAlloca(envType);

      storeEnvAllocaInArray(reduceArrAlloca, i, varAlloca);
    }
  }
}
