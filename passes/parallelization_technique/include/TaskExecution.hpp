#pragma once

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DerivedUser.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/IRBuilder.h"

#include <unordered_map>

using namespace std;

namespace llvm {

  struct TaskExecution {
    int order;
    Function *F;
    BasicBlock *entryBlock, *exitBlock;
    std::vector<BasicBlock *> loopExitBlocks;

    Value *envArg;
    Value *instanceIndexV;

    std::unordered_map<BasicBlock *, BasicBlock *> basicBlockClones;
    std::unordered_map<Instruction *, Instruction *> instructionClones;
    std::unordered_map<Value *, Value *> liveInClones;

    virtual void extractFuncArgs () = 0;
  };
}
