/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopEnvironment.hpp"
#include "noelle/core/LoopEnvironmentUser.hpp"

namespace llvm::noelle {

  class LoopEnvironmentBuilder {
   public:
    LoopEnvironmentBuilder (llvm::LLVMContext &CXT);

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
    void addVariableToEnvironment (uint64_t varIndex, Type *varType);

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

    LoopEnvironmentUser *getUser (int user) { return envUsers[user]; }
    int getNumUsers () { return envUsers.size(); }

    Value *getEnvVar (int ind) ;
    Value *getAccumulatedReducableEnvVar (int ind) ;
    Value *getReducableEnvVar (int ind, int reducerInd) ;
    bool isReduced (int ind) ;

    ~LoopEnvironmentBuilder ();

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
    std::unordered_map<int, Value *> envIndexToAccumulatedReducableVar;
    std::unordered_map<int, std::vector<Value *>> envIndexToReducableVar;
    std::unordered_map<int, AllocaInst *> envIndexToVectorOfReducableVar;
    int numReducers;

    /*
     * Information on a specific user (a function, stage, chunk, etc...)
     */
    std::vector<LoopEnvironmentUser *> envUsers;
  };

}
