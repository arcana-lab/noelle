#pragma once;

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"

#include "LoopEnvironment.hpp"

#include <unordered_map>
#include <vector>
#include <set>

namespace llvm {

  class EnvUserBuilder {
   public:
    EnvUserBuilder (LoopEnvironment &le);

    void setEnvArray (Value *envArr) { this->envArray = envArr; }
    void createEnvPtr (IRBuilder<> b, int envIndex);
    void createReducableEnvPtr (
      IRBuilder<> b,
      int envIndex,
      int reducerCount,
      Value *reducerIndV
    );

    void addPreEnvIndex (int ind) { preEnvIndices.insert(ind); }
    void addPostEnvIndex (int ind) { postEnvIndices.insert(ind); }

    iterator_range<std::set<int>::iterator> getPreEnvIndices() { 
      return make_range(preEnvIndices.begin(), preEnvIndices.end());
    }
    iterator_range<std::set<int>::iterator> getPostEnvIndices() { 
      return make_range(postEnvIndices.begin(), postEnvIndices.end());
    }

    Instruction *getEnvPtr (int ind) { return envIndexToPtr[ind]; }

   private:
    LoopEnvironment &LE;
    Value *envArray;

		/*
		 * Maps from environment index to load/stores
		 */
		std::unordered_map<int, Instruction *> envIndexToPtr;
    std::set<int> preEnvIndices;
    std::set<int> postEnvIndices;
  };

  class EnvBuilder {
   public:
    EnvBuilder (LoopEnvironment &LE, llvm::LLVMContext &CXT);
    ~EnvBuilder ();

    /*
     * Create an environment array, insert into the IRBuilder
     */
    void createEnvArray (IRBuilder<> builder);

    void createEnvUsers (int numUsers);

    void allocateEnvVariables (
      IRBuilder<>,
      std::set<int> &singleVarIndices,
      std::set<int> &reducableVarIndices,
      int reducerCount
    );

    void reduceLiveOutVariables (
      IRBuilder<>,
      std::unordered_map<int, int> &reducableBinaryOps,
      std::unordered_map<int, Value *> &initialValues,
      int reducerCount
    );

    /*
     * As all users of the environment konw its structure,
     *  pass around the equivalent of a void pointer
     */
    Value *getEnvArrayInt8Ptr () { return envArrayInt8Ptr; }
    Value *getEnvArray () { return envArray; }
    ArrayType *getEnvArrayTy () { return envArrayType; }

    EnvUserBuilder *getUser (int user) { return envUsers[user]; }

    Value *getEnvVar (int ind) { return envIndexToVar[ind]; }
    Value *getReducableEnvVar (int ind, int reducerInd) {
      return envIndexToReducableVar[ind][reducerInd];
    }

    bool isReduced (int ind) {
      return envIndexToReducableVar.find(ind) != envIndexToReducableVar.end();
    }

   private:
    LoopEnvironment &LE;

    /*
     * The environment array, owned by this builder
     */
    Value *envArray;
    Value *envArrayInt8Ptr;
    ArrayType *envArrayType;

    /*
     * The allocations for variables in the environment
     */
		std::unordered_map<int, Value *> envIndexToVar;
		std::unordered_map<int, std::vector<Value *>> envIndexToReducableVar;

    /*
     * Information on a specific user (a function, stage, chunk, etc...)
     */
    std::vector<EnvUserBuilder *> envUsers;
  };

}
