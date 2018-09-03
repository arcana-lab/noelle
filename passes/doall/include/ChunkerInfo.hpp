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
     * Clone of original IV loop, new outer loop
     */
    SCCAttrs *originIVAttrs;
    SimpleIVInfo cloneIVInfo;
    PHINode *cloneIV;
    PHINode *outerIV;

    /*
     * Utility functions
     */
    BasicBlock *createChunkerBB () {
      auto &cxt = f->getContext();
      return BasicBlock::Create(cxt, "", f);
    }

    Value *fetchClone (Value *original) {
      if (isa<ConstantInt>(original)) return original;
      if (preEnvMap.find(original) != preEnvMap.end()) {
        return preEnvMap[original];
      }
      return innerValMap[cast<Instruction>(original)];
    }
  };
}

