/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"

using namespace std;
using namespace llvm;

namespace llvm::noelle {

  class Task {
    public:

      Task (
        uint32_t ID,
        FunctionType *taskSignature,
        Module &M
        );

      /*
       * IDs
       */

      uint32_t getID (void) const ;

      Value * getTaskInstanceID (void) const ;


      /*
       * Live-in values.
       */

      bool isAnOriginalLiveIn (Value *v) const ;

      Value * getCloneOfOriginalLiveIn (Value *o) const ;

      std::unordered_set<Value *> getOriginalLiveIns (void) const ;

      void addLiveIn (Value *original, Value *internal) ;

      /*
       * Live-out instructions
       */
      bool doesOriginalLiveOutHaveManyClones (Instruction *I) const ;

      std::unordered_set<Instruction *> getClonesOfOriginalLiveOut (Instruction *I) const ;

      void addLiveOut (Instruction *original, Instruction *internal) ;

      void removeLiveOut (Instruction *original, Instruction *removed) ;

      /*
       * Instructions
       */

      bool isAnOriginalInstruction (Instruction *i) const ;

      Instruction * getCloneOfOriginalInstruction (Instruction *o) const ;

      void addInstruction (Instruction *original, Instruction *internal) ;

      std::unordered_set<Instruction *> getOriginalInstructions (void) const ;

      Instruction * cloneAndAddInstruction (Instruction *original);

      void removeOriginalInstruction (Instruction *o);


      /*
       * Basic blocks
       */

      bool isAnOriginalBasicBlock (BasicBlock *o) const ;

      BasicBlock * getCloneOfOriginalBasicBlock (BasicBlock *o) const ;

      std::unordered_set<BasicBlock *> getOriginalBasicBlocks (void) const ;

      void addBasicBlock (BasicBlock *original, BasicBlock *internal) ;

      BasicBlock * addBasicBlockStub (BasicBlock *original);

      BasicBlock * cloneAndAddBasicBlock (BasicBlock *original);

      void removeOriginalBasicBlock (BasicBlock *b);

      BasicBlock * getEntry (void) const ;

      BasicBlock * getExit (void) const ;

      uint32_t getNumberOfLastBlocks (void) const ;

      BasicBlock * getLastBlock (uint32_t blockID) const ;

      void tagBasicBlockAsLastBlock (BasicBlock *b) ;


      /*
       * Body
       */

      Function * getTaskBody (void) const ;


      /*
       * Dependences with the outside code
       */
      Value * getEnvironment (void) const ;

      virtual void extractFuncArgs (void) = 0;


    protected:
      uint32_t ID;
      Function *F;

      /*
       * There is a one-to-one mapping between the original live in value and a pointer
       * to the environment where that original live in value is stored for use by the task
       */
      std::unordered_map<Value *, Value *> liveInClones;

      /*
       * With few exceptions, the clone of the live out value is used directly, stored
       * into the environment for use after the task executes. When that value is duplicated
       * by tasks doing more complicated transformations, this structure holds the mapping
       * between the original live out instruction and all its duplicates
       */
      std::unordered_map<Instruction *, std::unordered_set<Instruction *>> liveOutClones;

      /*
       * This is a one-to-one mapping between the original loop's structure and the
       * task's cloned loop structure
       * TODO: Provide a one-to-many mapping for use by more complex transformations
       */
      std::unordered_map<BasicBlock *, BasicBlock *> basicBlockClones;
      std::unordered_map<Instruction *, Instruction *> instructionClones;

      Value *instanceIndexV;
      Value *envArg;
      BasicBlock *entryBlock;
      BasicBlock *exitBlock;
      std::vector<BasicBlock *> lastBlocks;

      LLVMContext & getLLVMContext (void) const ;
  };

}
