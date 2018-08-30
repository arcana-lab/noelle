#pragma once;

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include <unordered_map>

namespace llvm {
  struct ChunkerInfo {
    Function *f;
    BasicBlock *entryBlock, *exitBlock;

    /*
     * Chunking (outer) loop header/latch
     */
    BasicBlock *chHeader, *chLatch;

    /*
     * Chunking function arguments
     */
    Value *envArgVal, *coreArgVal;
    Value *numCoresArgVal, *chunkSizeArgVal;

    /*
     * Map from original to clone:
     *  inner loop values
     *  environment values
     *  inner loop bb structure
     */
    unordered_map<Instruction *, Instruction *> innerValMap;
    unordered_map<Value *, Value *> preEnvMap;
    unordered_map<BasicBlock *, BasicBlock *> innerBBMap;

    /*
     * Inner loop IV values
     */
    BranchInst *originHeaderBr;
    CmpInst *originCmp;
    CmpInst::Predicate strictPredicate; 
    PHINode *originIV;
    Value *maxIV;
    int originCmpPHIIndex, originCmpMaxIndex;

    Value *stepperIV;
    ConstantInt *originStepSize;
    int stepSizeIVIndex;

    Value *starterIV;
    int startValIVIndex;

    /*
     * Outer loop values
     */
    Value *chIV;

    /*
     * Utility values/functions
     */
    Value *zeroV;

    BasicBlock *createChunkerBB () {
      auto &cxt = f->getContext();
      return BasicBlock::Create(cxt, "", f);
    }

  };
}

