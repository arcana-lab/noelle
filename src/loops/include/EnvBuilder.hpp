/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once;

#include "SystemHeaders.hpp"
#include "LoopEnvironment.hpp"

namespace llvm {

  class EnvUserBuilder {
   public:
    EnvUserBuilder ();
    ~EnvUserBuilder ();

    void setEnvArray (Value *envArr) { this->envArray = envArr; }
    void createEnvPtr (IRBuilder<> b, int envIndex, Type *type);
    void createReducableEnvPtr (
      IRBuilder<> b,
      int envIndex,
      Type *type,
      int reducerCount,
      Value *reducerIndV
    );

    void addLiveInIndex (int ind) { liveInInds.insert(ind); }
    void addLiveOutIndex (int ind) { liveOutInds.insert(ind); }

    iterator_range<std::set<int>::iterator> getEnvIndicesOfLiveInVars() { 
      return make_range(liveInInds.begin(), liveInInds.end());
    }
    iterator_range<std::set<int>::iterator> getEnvIndicesOfLiveOutVars() { 
      return make_range(liveOutInds.begin(), liveOutInds.end());
    }

    Instruction *getEnvPtr (int ind) { return envIndexToPtr[ind]; }

   private:
    Value *envArray;

		/*
		 * Maps from environment index to load/stores
		 */
    std::unordered_map<int, Instruction *> envIndexToPtr;
    std::set<int> liveInInds;
    std::set<int> liveOutInds;
  };

  class EnvBuilder {
   public:
    EnvBuilder (llvm::LLVMContext &CXT);
    ~EnvBuilder ();

    /*
     * Create environment users and designate variable types
     */
    void createEnvUsers (int numUsers);
    void createEnvVariables (
      std::vector<Type *> &varTypes,
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
    BasicBlock * reduceLiveOutVariables (
      BasicBlock *bb,
      IRBuilder<>,
      std::unordered_map<int, int> &reducableBinaryOps,
      std::unordered_map<int, Value *> &initialValues,
      Value *numberOfThreadsExecuted
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

    /*
     * The environment array, owned by this builder
     */
    LLVMContext &CXT;
    Value *envArray;
    Value *envArrayInt8Ptr;

    /*
     * The environment variable types and their allocations
     */
    int envSize;
    ArrayType *envArrayType;
    std::vector<Type *> envTypes;
    std::unordered_map<int, Value *> envIndexToVar;
    std::unordered_map<int, std::vector<Value *>> envIndexToReducableVar;
    std::unordered_map<int, AllocaInst *> envIndexToVectorOfReducableVar;
    int numReducers;

    /*
     * Information on a specific user (a function, stage, chunk, etc...)
     */
    std::vector<EnvUserBuilder *> envUsers;
  };

}
