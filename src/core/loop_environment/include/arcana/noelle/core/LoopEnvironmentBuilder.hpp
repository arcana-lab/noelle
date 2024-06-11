/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef NOELLE_SRC_CORE_LOOP_ENVIRONMENT_LOOPENVIRONMENTBUILDER_H_
#define NOELLE_SRC_CORE_LOOP_ENVIRONMENT_LOOPENVIRONMENTBUILDER_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "noelle/core/BinaryReductionSCC.hpp"
#include "arcana/noelle/core/LoopEnvironment.hpp"
#include "arcana/noelle/core/LoopEnvironmentUser.hpp"

namespace arcana::noelle {

class LoopEnvironmentBuilder {
public:
  LoopEnvironmentBuilder(LLVMContext &cxt,
                         LoopEnvironment *env,
                         uint64_t numberOfUsers);

  LoopEnvironmentBuilder(
      LLVMContext &cxt,
      LoopEnvironment *env,
      std::function<bool(uint32_t variableID, bool isLiveOut)>
          shouldThisVariableBeReduced,
      uint64_t reducerCount,
      uint64_t numberOfUsers);

  LoopEnvironmentBuilder(
      LLVMContext &cxt,
      LoopEnvironment *env,
      std::function<bool(uint32_t variableID, bool isLiveOut)>
          shouldThisVariableBeReduced,
      std::function<bool(uint32_t variableID, bool isLiveOut)>
          shouldThisVariableBeSkipped,
      uint64_t reducerCount,
      uint64_t numberOfUsers);

  LoopEnvironmentBuilder(LLVMContext &CXT,
                         const std::vector<Type *> &varTypes,
                         const std::set<uint32_t> &singleVarIDs,
                         const std::set<uint32_t> &reducableVarIDs,
                         uint64_t reducerCount,
                         uint64_t numberOfUsers);

  LoopEnvironmentBuilder() = delete;

  virtual void addVariableToEnvironment(uint64_t varID, Type *varType);

  /*
   * Generate code to create environment array/variable allocations
   */
  virtual void allocateEnvironmentArray(IRBuilder<> &builder);
  virtual void generateEnvVariables(IRBuilder<> &builder);

  /*
   * Reduce live out variables given binary operators to reduce
   * with and initial values to start at
   */
  virtual BasicBlock *reduceLiveOutVariables(
      BasicBlock *bb,
      IRBuilder<> &builder,
      const std::unordered_map<uint32_t, BinaryReductionSCC *> &reductions,
      Value *numberOfThreadsExecuted,
      std::function<Value *(ReductionSCC *scc)> castingInitialValue);

  /*
   * As all users of the environment know its structure, pass around the
   * equivalent of a void pointer
   */
  virtual Value *getEnvironmentArrayVoidPtr(void) const;
  virtual Value *getEnvironmentArray(void) const;
  virtual ArrayType *getEnvironmentArrayType(void) const;

  virtual LoopEnvironmentUser *getUser(uint32_t user) const;
  virtual uint32_t getNumberOfUsers(void) const;

  virtual Value *getEnvironmentVariable(uint32_t id) const;
  virtual uint32_t getIndexOfEnvironmentVariable(uint32_t id) const;
  virtual bool isIncludedEnvironmentVariable(uint32_t id) const;
  virtual Value *getAccumulatedReducedEnvironmentVariable(uint32_t id) const;
  virtual Value *getReducedEnvironmentVariable(uint32_t id,
                                               uint32_t reducerInd) const;
  virtual bool hasVariableBeenReduced(uint32_t id) const;

  virtual ~LoopEnvironmentBuilder();

protected:
  /*
   * The environment array, owned by this builder
   */
  LLVMContext &CXT;
  Value *envArray;
  Value *envArrayInt8Ptr;

  /*
   * Map and reverse map from envID to index
   */
  std::unordered_map<uint32_t, uint32_t> envIDToIndex;
  std::unordered_map<uint32_t, uint32_t> indexToEnvID;

  /*
   * The environment variable types and their allocations
   */
  uint64_t envSize;
  ArrayType *envArrayType;
  std::vector<Type *> envTypes;
  std::unordered_map<uint32_t, Value *> envIndexToVar;
  std::unordered_map<uint32_t, Value *> envIndexToAccumulatedReducableVar;
  std::unordered_map<uint32_t, std::vector<Value *>> envIndexToReducableVar;
  std::unordered_map<uint32_t, AllocaInst *> envIndexToVectorOfReducableVar;
  uint64_t numReducers;

  /*
   * Information on a specific user (a function, stage, chunk, etc...)
   */
  std::vector<LoopEnvironmentUser *> envUsers;

  virtual void initializeBuilder(const std::vector<Type *> &varTypes,
                                 const std::set<uint32_t> &singleVarIDs,
                                 const std::set<uint32_t> &reducableVarIDs,
                                 uint64_t reducerCount,
                                 uint64_t numberOfUsers);

  virtual void createUsers(uint32_t numUsers);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_ENVIRONMENT_LOOPENVIRONMENTBUILDER_H_
