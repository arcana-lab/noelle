/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/iterator_range.h"
#include <queue>
#include <set>

using namespace llvm;

namespace llvm {

  enum class AllocAAVerbosity { Disabled, Minimal, Maximal };

  struct AllocAA : public ModulePass {
    public:
      static char ID;

      AllocAA () : ModulePass{ID} {}

      bool doInitialization (Module &M) override ;

      bool runOnModule (Module &M) override ;

      void getAnalysisUsage(AnalysisUsage &AU) const override ;

      std::pair<Value *, GetElementPtrInst *> getPrimitiveArrayAccess (Value *V);

      bool areGEPIndicesConstantOrIV (GetElementPtrInst *gep);
      bool areIdenticalGEPAccessesInSameLoop (GetElementPtrInst *gep1, GetElementPtrInst *gep2);

      bool isReadOnly (StringRef functionName);
      bool isMemoryless (StringRef functionName);

    private:
      std::set<Function *> CGUnderMain;
      // TODO: These should become objects representing the full usage of these allocated arrays
      std::set<CallInst *> allocatorCalls;
      std::set<std::string> readOnlyFunctionNames, allocatorFunctionNames, memorylessFunctionNames;
      std::set<GlobalValue *> primitiveArrayGlobals;
      std::set<Instruction *> primitiveArrayLocals;
      AllocAAVerbosity verbose;

      // TODO: Find a way to extract this into a helper module for all passes in the PDG project
      void collectCGUnderFunctionMain (Module &M, CallGraph &callGraph);
      void collectAllocations (Module &M, CallGraph &callGraph);
      void collectFunctionCallsTo (
        CallGraph &callGraph,
        std::set<Function *> &called,
        std::set<CallInst *> &calls
      );

      void collectMemorylessFunctions (Module &M);

      bool collectUserInstructions (Value *V, std::set<Instruction *> &userInstructions);
      void collectPrimitiveArrayValues (Module &M);
      bool isPrimitiveArray (Value *V, std::set<Instruction *> &userInstructions);
      bool isPrimitiveArrayPointer (Value *V, std::set<Instruction *> &userInstructions);
      bool doesValueNotEscape (std::set<Instruction *> checked, Instruction *I);

      Value *getPrimitiveArray (Value *V);
      Value *getLocalPrimitiveArray (Value *V);
      Value *getGlobalValuePrimitiveArray (Value *V);
      Value *getMemoryPointerOperand (Value *V);
  };
}
