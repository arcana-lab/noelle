/*
 * Copyright 2020 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "OutlinerPass.hpp"
#include "llvm/Pass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/IR/BasicBlock.h"

namespace llvm::noelle {

  Outliner::Outliner () {
  
    return ;
  }

  int Outliner::numOutlines { 0 };

  Function * Outliner::outline (std::unordered_set<Instruction *> const & instructionsToOutline, Instruction *injectCallJustBeforeThis, AssumptionCache * AC){
    //TODO
    //1. Assume that this is for reduction? and it already contains the instruction that we want to reduce
    //2. Assume all instructions belong to the same basic block ? 
    //3. Split basic block into three BBs  - can we do that? split it twice ??? Brian?
    //4. move instruction to a separate basic block
    //5. add this block to ArrayRef
    //6. call outline with ArrayRef of basicblocks

    BasicBlock* sourceBb = (*(instructionsToOutline.begin()))->getParent();
    Function* sourceF = sourceBb->getParent();
    BasicBlock* outBb = sourceBb->splitBasicBlock((*(instructionsToOutline.begin())), "outlinerBB");
    //now all the instructions were added to the new basic block (even the ones that are not in our set)
    //so we have to split it again but first find the first instruction that doesn't belong to this set?

    for(auto &i : *outBb){
      if(instructionsToOutline.find(&i) == instructionsToOutline.end()){ //this is the first instruction not in the list so we need to start a new basic block here
        BasicBlock* remainderBb = sourceBb->splitBasicBlock(&i, "remainderBB");
        break;
      }
    }
    
    //create ArrayRef
    ArrayRef<BasicBlock *> basicBlocksToOutline = {outBb};
    
    return Outliner::outline(basicBlocksToOutline, injectCallJustBeforeThis, sourceF, AC);
  }

  Function * Outliner::outline (ArrayRef<BasicBlock *> const & basicBlocksToOutline, Instruction *injectCallJustBeforeThis, Function* sourceF, AssumptionCache * AC){
    //TODO
    //1. Collect data about basicblocks
    // - check dependencies that enter basic blocks from the outside - input variables 
    // - define input variables which are not modified inside basic block (pass by value)
    // - define input variables which are modified inside basic block (pass by reference)
    //2. Create noinline function
    //3. Use llvm to rewire all variables
    //4. Return new function

    // Verify that basic blocks are single entry
    // Single exit?

    auto DT = DominatorTree(*sourceF);
    CodeExtractor CE(basicBlocksToOutline, &DT, /* AggregateArgs */ false, /* BFI */ nullptr,
                   /* BPI */ nullptr, AC, /* AllowVarArgs */ false,
                   /* AllowAlloca */ false,
                   /* Suffix */ "outliner."  + std::to_string(numOutlines));

    if (basicBlocksToOutline.empty()) { errs() << "It's empty? :(\n"; }
    if ((*basicBlocksToOutline.begin())->getParent()->getFunctionType()->isVarArg()) {
      errs() << "its a var arg\n";
    }

    errs() << "is eligible? " << CE.isEligible() << '\n';
    // Extract code to new function
    if (Function* newF = CE.extractCodeRegion()) {
      // Get Caller and set no inline
      User *U = *newF->user_begin();
      CallInst *CI = cast<CallInst>(U);
      CI->setIsNoInline();
      numOutlines++;
      // Return the new Function
      return newF;
    }
    errs() << "Failed to extract\n";
    return nullptr;
  }

}
