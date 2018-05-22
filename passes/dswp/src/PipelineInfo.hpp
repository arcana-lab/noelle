#pragma once

#include "llvm/IR/Instructions.h"
#include "PDG.hpp"
#include "SCC.hpp"

#include <unordered_map>

using namespace std;
using namespace llvm;

namespace llvm {

	struct EnvInfo {
		std::vector<Value *> externalDependents;
		std::set<int> preLoopExternals;
		std::set<int> postLoopExternals;

		bool hasRetValue;
		Type *retType;

		/*
		 * One per external dependent + one to track exit block (+ one if has loop-internal returns)
		 */
		int envSize() { return externalDependents.size() + 1 + (hasRetValue ? 1 : 0); }

		int indexOfExitBlock() { return externalDependents.size(); }
		int indexOfRetVal() { return externalDependents.size() + 1; }
	};

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
            bitLength = dependentType->getPrimitiveSizeInBits();
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
		Function *sccStage;
		int order;

		/*
		 * Original loops' scc and basic blocks
		 */
		SCC *scc;
		std::set<BasicBlock *> sccBBs;

		/*
		 * New basic blocks for the stage function
		 */
		BasicBlock *entryBlock, *abortBlock, *exitBlock;
		std::vector<BasicBlock *> loopExitBlocks;
		BasicBlock *prologueBlock, *epilogueBlock;
		unordered_map<int, BasicBlock *> controlToSwitchBlock;
		unordered_map<BasicBlock *, BasicBlock *> switchToSCCEntry;
		unordered_map<BasicBlock *, std::set<BasicBlock *>> switchToIntermediates;

		/*
		 * Maps original loop instructions to clones 
		 */
		unordered_map<Instruction *, Instruction *> iCloneMap;

		/*
		 * Map original to clone basic blocks for: scc execution, predecessors, and successors
		 */
		unordered_map<BasicBlock*, BasicBlock*> sccBBCloneMap;

		/*
		 * Maps from instructions within loop to environment indices
		 */
        unordered_map<Instruction *, int> incomingToEnvMap, outgoingToEnvMap;

        /*
         * Maps from instructions outside of loop to environment indices
         */
        unordered_map<Value *, int> externalToEnvMap;

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
		Value *envAlloca;
	};
}