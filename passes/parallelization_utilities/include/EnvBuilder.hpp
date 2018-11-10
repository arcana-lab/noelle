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
    ~EnvUserBuilder ();

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
     * Create environment users and designate variable types
     */
    void createEnvUsers (int numUsers);
    void createEnvVariables (
      std::set<int> &singleVarIndices,
      std::set<int> &reducableVarIndices,
      int reducerCount
    );

    /*
     * Generate code to create environment array/variable allocations
     */
    void generateEnvArray (IRBuilder<> builder);
    void generateEnvVariables (IRBuilder<>);

    /*
     * Reduce live out variables given binary operators to reduce
     * with and initial values to start at
     */
    void reduceLiveOutVariables (
      IRBuilder<>,
      std::unordered_map<int, int> &reducableBinaryOps,
      std::unordered_map<int, Value *> &initialValues
    );

    /*
     * As all users of the environment konw its structure,
     *  pass around the equivalent of a void pointer
     */
    Value *getEnvArrayInt8Ptr () ;
    Value *getEnvArray () ;
    ArrayType *getEnvArrayTy () { return envArrayType; }

    EnvUserBuilder *getUser (int user) { return envUsers[user]; }
    int getNumUsers () { return envUsers.size(); }

    Value *getEnvVar (int ind) ;
    Value *getReducableEnvVar (int ind, int reducerInd) ;
    bool isReduced (int ind) ;

   private:
    LoopEnvironment &LE;

    /*
     * The environment array, owned by this builder
     */
    Value *envArray;
    Value *envArrayInt8Ptr;
    ArrayType *envArrayType;

    /*
     * The environment variable types and their allocations
     */
		std::unordered_map<int, Value *> envIndexToVar;
		std::unordered_map<int, std::vector<Value *>> envIndexToReducableVar;
    int numReducers;

    /*
     * Information on a specific user (a function, stage, chunk, etc...)
     */
    std::vector<EnvUserBuilder *> envUsers;
  };

}
