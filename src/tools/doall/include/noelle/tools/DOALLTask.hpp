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
#ifndef NOELLE_SRC_TOOLS_DOALL_DOALLTASK_H_
#define NOELLE_SRC_TOOLS_DOALL_DOALLTASK_H_

#include "noelle/core/Task.hpp"
#include "noelle/core/SCCDAGAttrs.hpp"

namespace arcana::noelle {

class DOALLTask : public Task {
public:
  DOALLTask(FunctionType *taskSignature, Module &M);
  DOALLTask(FunctionType *taskSignature,
            Module &M,
            const std::string &taskFunctionNameToUse);

  /*
   * Inner loop header/latch
   */
  BasicBlock *outermostLoopHeader, *outermostLoopLatch;

  /*
   * Chunking function specific arguments
   */
  Value *taskInstanceID, *numTaskInstances, *chunkSizeArg;

  /*
   * Clone of original IV loop, new outer loop
   */
  GenericSCC *originalIVAttrs;
  PHINode *cloneOfOriginalIV;
  CmpInst *cloneOfOriginalCmp;
  BranchInst *cloneOfOriginalBr;
  PHINode *outermostLoopIV;

protected:
  void initializeTask(Function *F);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_DOALL_DOALLTASK_H_
