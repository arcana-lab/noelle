#pragma once

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "PDG.hpp"
#include "SCC.hpp"

#include <unordered_map>

using namespace std;

namespace llvm {

  struct QueueInfo {
    int fromStage, toStage;
    Type *dependentType;
    int bitLength;

    Instruction * producer;
    std::set<Instruction *> consumers;
    unordered_map<Instruction *, int> consumerToPushIndex;

    QueueInfo(Instruction *p, Instruction *c, Type *type) : producer{p}, dependentType{type}
    {
      consumers.insert(c);
      if (dependentType->isPointerTy()) {
        bitLength = DataLayout(p->getModule()).getTypeAllocSize(dependentType) * 8;
      } else {
        bitLength = dependentType->getPrimitiveSizeInBits();
      }
    }
  };

  struct QueueInstrs
  {
    Value *queuePtr;
    Value *queueCall;
    Value *alloca;
    Value *allocaCast;
    Value *load;
  };

  struct StageInfo {
    StageInfo(int ord) : order{ord} {}

    Function *sccStage;
    int order;

    /*
     * Original loops' relevant structures
     */
    std::set<SCC *> stageSCCs;
    std::set<SCC *> removableSCCs;
    std::set<TerminatorInst *> usedCondBrs;

    /*
     * New basic blocks for the stage function
     */
    BasicBlock *entryBlock, *abortBlock, *exitBlock;
    std::vector<BasicBlock *> loopExitBlocks;
    BasicBlock *prologueBlock, *epilogueBlock;

    /*
     * Maps original loop instructions to clones 
     */
    unordered_map<Instruction *, Instruction *> iCloneMap;

    /*
     * Map original to clone basic blocks for: scc execution, predecessors, and successors
     */
    unordered_map<BasicBlock*, BasicBlock*> sccBBCloneMap;

    /*
     * Maps from producer to the queues they push to
     */
    unordered_map<Instruction *, std::set<int>> producerToQueues;

    /*
     * Maps from other stage's producer to the pop value queue of this stage
     */
    unordered_map<Instruction *, int> producedPopQueue;

    /*
     * Stores queue indices and pointers for the stage
     */
    std::set<int> pushValueQueues, popValueQueues;

    /*
     * Stores information on queue/env usage within stage
     */
    unordered_map<int, std::unique_ptr<QueueInstrs>> queueInstrMap;
    unordered_map<int, Instruction *> envLoadMap;
  };
}
