#pragma once

#include "llvm/IR/Instructions.h"
#include "PDG.hpp"
#include "SCC.hpp"

#include <unordered_map>

using namespace std;
using namespace llvm;

namespace llvm {

	struct EnvInfo {
		std::vector<Value *> envProducers;
		unordered_map<Value *, int> producerIndexMap;
		unordered_map<Value *, set<Value *>> prodConsumers;
		std::set<int> preLoopEnv;
		std::set<int> postLoopEnv;

		Type *exitBlockType;

		/*
		 * One per external dependent + one to track exit block
		 */
		int envSize () { return envProducers.size() + 1; }
		int indexOfExitBlock () { return envProducers.size(); }

		Type *typeOfEnv (int index)
		{
			if (index < envProducers.size()) return envProducers[index]->getType();
			return exitBlockType;
		}

		void addProducer (Value *producer, bool preLoop)
		{
			auto envIndex = envProducers.size();
			envProducers.push_back(producer);
			producerIndexMap[producer] = envIndex;
			if (preLoop) preLoopEnv.insert(envIndex);
			else postLoopEnv.insert(envIndex);
		}
		void addPreLoopProducer (Value *producer) { addProducer(producer, true); }
		void addPostLoopProducer (Value *producer) { addProducer(producer, false); }

		bool isPreLoopEnv(Value *val)
		{
			return producerIndexMap.find(val) != producerIndexMap.end() && preLoopEnv.find(producerIndexMap[val]) != preLoopEnv.end();
		}
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
		unordered_map<Instruction *, int> outgoingEnvs;

		/*
		 * Stores incoming environment indices
		 */
		set<int> incomingEnvs;

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