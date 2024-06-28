/*
 * Copyright 2016 - 2024  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_ALLOC_AA_ALLOCAA_H_
#define NOELLE_SRC_CORE_ALLOC_AA_ALLOCAA_H_

#include "arcana/noelle/core/SystemHeaders.hpp"

namespace arcana::noelle {

class AllocAA {
public:
  AllocAA(Module &M,
          std::function<llvm::ScalarEvolution &(Function &F)> getSCEV,
          std::function<llvm::LoopInfo &(Function &F)> getLoopInfo,
          std::function<llvm::CallGraph &(void)> getCallGraph);

  std::pair<Value *, GetElementPtrInst *> getPrimitiveArrayAccess(Value *V);

  bool areGEPIndicesConstantOrIV(GetElementPtrInst *gep);
  bool areIdenticalGEPAccessesInSameLoop(GetElementPtrInst *gep1,
                                         GetElementPtrInst *gep2);

  bool canPointToTheSameObject(Value *p1, Value *p2);

  bool isReadOnly(StringRef functionName);
  bool isMemoryless(StringRef functionName);

private:
  Module &M;
  std::function<llvm::ScalarEvolution &(Function &F)> getSCEV;
  std::function<llvm::LoopInfo &(Function &F)> getLoopInfo;
  std::function<llvm::CallGraph &(void)> getCallGraph;
  std::set<Function *> CGUnderMain;
  // TODO: These should become objects representing the full usage of these
  // allocated arrays
  std::set<CallInst *> allocatorCalls;
  std::set<std::string> readOnlyFunctionNames, allocatorFunctionNames,
      memorylessFunctionNames;
  std::set<GlobalValue *> primitiveArrayGlobals;
  std::set<Instruction *> primitiveArrayLocals;

  // TODO: Find a way to extract this into a helper module for all passes in the
  // PDG project
  void collectCGUnderFunctionMain(Module &M, CallGraph &callGraph);
  void collectAllocations(Module &M, CallGraph &callGraph);
  void collectFunctionCallsTo(CallGraph &callGraph,
                              std::set<Function *> &called,
                              std::set<CallInst *> &calls);

  void collectMemorylessFunctions(Module &M);

  bool collectUserInstructions(Value *V,
                               std::set<Instruction *> &userInstructions);
  void collectPrimitiveArrayValues(Module &M);
  bool isPrimitiveArray(Value *V, std::set<Instruction *> &userInstructions);
  bool isPrimitiveArrayPointer(Value *V,
                               std::set<Instruction *> &userInstructions);
  bool doesValueNotEscape(std::set<Instruction *> checked, Instruction *I);

  Value *getPrimitiveArray(Value *V);
  Value *getLocalPrimitiveArray(Value *V);
  Value *getGlobalValuePrimitiveArray(Value *V);
  Value *getMemoryPointerOperand(Value *V);

  Value *getBasePointer(Value *p);
  bool canPointToTheSameObject_ArgumentAttributes(Value *p1, Value *p2);
  bool canPointToTheSameObject_Globals(Value *p1, Value *p2);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_ALLOC_AA_ALLOCAA_H_
