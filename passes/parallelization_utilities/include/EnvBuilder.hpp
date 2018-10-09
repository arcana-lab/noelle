#pragma once;

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "LoopEnvironment.h"

#include <unordered_map>
#include <vector>
#include <set>

namespace llvm {

  class EnvUserBuilder {
   public:
    EnvUserBuilder (LoopEnvironment &le, Value *envArr);

    createEnvPtrTo (std::set<int> &userEnvIndices);
    createEnvPtrTo (int envIndex);

   private:
    LoopEnvironment &LE;

    /*
     * User's pointer to environment array
     */
    Value *envArray;

		/*
		 * Maps from environment index to load/stores
		 */
		std::unordered_map<int, Instruction *> envIndexToPtr;
  };

  class EnvBuilder {
   public:
    EnvBuilder (LoopEnvironment &LE);

    /*
     * Create an environment array, insert into the IRBuilder
     */
    createEnvArray (IRBuilder<> builder);

    allocateEnvVariables (IRBuilder<>, std::set<int> singleVarIndices, std::set<int> reducableVarIndices);

    /*
     * As all users of the environment konw its structure,
     *  pass around the equivalent of a void pointer
     */
    getEnvArrayInt8Ptr () { return envArrayInt8Ptr; }

   private:
    LoopEnvironment &LE;

    /*
     * The environment array, owned by this builder
     */
    Value *envArray;
    Value *envArrayInt8Ptr;
    ArrayType *envArrayType;

    /*
     * Information on a specific user (a function, stage, chunk, etc...)
     */
    std::set<EnvUserBuilder> envUser;

    /*
     * The builder for the original loop's function
     */
    EnvUserBuilder *loopFunctionEnvBuilder;

    /*
     * Maps from environment index to reduced value
     */
    // NOTE(angelo): Maybe not needed
    // std::unordered_map<int, Instruction *> envIndexToReduced;
  };

}
