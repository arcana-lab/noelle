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
		Type *dependentType;
		int dependentBitSize;
		int byteLength;

        Instruction * producer;
        std::set<Instruction *> consumers;
        unordered_map<Instruction *, int> consumerToPushIndex;

        QueueInfo(Instruction *p, Instruction *c, Type *type) : producer{p}, dependentType{type}
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

	struct QueueInstrs
	{
		Value *queuePtr;
		Value *queueCall;
		Value *alloca;
		Value *allocaCast;
		Value *load;
	};

	struct LocalSwitch
	{
		unordered_map<Instruction *, int> producerToPushIndex;
	};

	struct StageInfo {
		Function *sccStage;
		int order;

		/*
		 * Original loops' scc and basic blocks
		 */
		SCC *scc;
		std::set<BasicBlock *> sccBBs;
		std::set<BasicBlock *> sccEntries;
		std::set<BasicBlock *> sccExits;
		unordered_map<BasicBlock *, int> exitBBToIndex;

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
         * Maps from producer to the queues they push to
         * Maps from consumer to the queues they pop from
         */
        unordered_map<Instruction *, std::set<int>> producerToQueues;
        unordered_map<Instruction *, std::set<int>> consumerToQueues;

        /*
         * Stores queue indices and pointers for the stage
         */
        std::set<int> pushValueQueues, popValueQueues;
        std::set<int> pushSwitchQueues, popSwitchQueues;
        std::set<int> pushControlQueues, popControlQueues;

        /*
         * Stores information on queue/switch usage within stage
         */
        unordered_map<int, std::unique_ptr<QueueInstrs>> queueInstrMap;
        unordered_map<Instruction *, std::unique_ptr<LocalSwitch>> consumerToLocalSwitches;
	};
}