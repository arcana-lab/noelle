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
	};

	struct QueueInfo {
		int fromStage, toStage;
		bool isControl;
		Type *dependentType;
		int dependentBitSize;
		int byteLength;

        Instruction * producer;
        std::set<Instruction *> consumers;

        QueueInfo(Instruction *p, Instruction *c) : producer{p}
        {
            consumers.insert(c);
			isControl = isa<TerminatorInst>(p);
			dependentType = isControl ? IntegerType::get(cast<Value>(p)->getContext(), 1) : p->getType();
			calculateByteReqs();
        }

        QueueInfo(Instruction *p, Instruction *c, Type *type, bool control) : producer{p}, dependentType{type}, isControl{control}
        {
            consumers.insert(c);
        	calculateByteReqs();
        }

        void calculateByteReqs()
        {
			// Calculates number of bytes needed to fit the number of bits
			dependentBitSize = dependentType->getPrimitiveSizeInBits();
			byteLength = (dependentBitSize + (8 - (dependentBitSize % 8))) / 8;
        }
	};

	struct StageInfo {
		Function *sccStage;
		int order;

		/*
		 * Original loops' scc and basic blocks
		 */
		SCC *scc;
		unordered_map<BasicBlock *, std::set<Instruction *>> bbToSCCInstsMap;
		std::set<BasicBlock *> sccBBs;
		std::set<BasicBlock *> sccEntries;

		/*
		 * New basic blocks for the stage function
		 */
		BasicBlock *entryBlock, *exitBlock;
		BasicBlock *prologueBlock, *epilogueBlock;
		BasicBlock *switcherBlock;

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
         * Maps from producer to the queue they push to
         * Maps from consumer to their producers
         */
        std::map<Instruction *, int> producerToValueOrControlQueueMap;
        std::map<Instruction *, int> producerToSwitchQueueMap;
        unordered_map<Instruction *, set<int>> consumerToQueuesMap;
        /*
         * Stores queue indices and pointers for the stage
         */
        std::set<int> pushValueQueues, popValueQueues;
        std::set<int> pushSwitchQueues, popSwitchQueues;
        std::set<int> pushControlQueues, popControlQueues;

        unordered_map<int, Value *> queueIndexToPointer;
        unordered_map<int, Value *> queueIndexToPopLoad;
	};
}