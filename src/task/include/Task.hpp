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

namespace llvm {

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

      bool isAnOriginalBasicBlock (BasicBlock *o) const ;


      /*
       * Instructions
       */

      bool isAnOriginalInstruction (Instruction *i) const ;

      Instruction * getCloneOfOriginalInstruction (Instruction *o) const ;


      /*
       * Basic blocks
       */

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

      virtual void extractFuncArgs () = 0;


      std::unordered_map<Instruction *, Instruction *> instructionClones;

    protected:
      uint32_t ID;
      Function *F;
      std::unordered_map<Value *, Value *> liveInClones;
      std::unordered_map<BasicBlock *, BasicBlock *> basicBlockClones;
      Value *instanceIndexV;
      Value *envArg;
      BasicBlock *entryBlock;
      BasicBlock *exitBlock;
      std::vector<BasicBlock *> lastBlocks;

      LLVMContext & getLLVMContext (void) const ;
  };

}
