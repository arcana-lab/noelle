#pragma once

#include "llvm/IR/Instructions.h"
#include "PDG.hpp"
#include "SCC.hpp"

#include <unordered_map>

using namespace std;
using namespace llvm;

namespace llvm {

	struct IncomingPipelineInfo {
		/*
		 * Storage and cast of the incoming value
		 */
        Instruction * popStorage;
        Value * popCast;

        /*
         * Queue pop followed by load of pop storage's value
         */
		CallInst * popQueueCall;
        LoadInst * loadStorage;

        /*
         * List of instructions using the pop storage's value
         */
        std::vector<Instruction *> userInstructions;
	};

	struct OutgoingPipelineInfo {
		/*
		 * Calculation, store, and cast of the outgoing value and a pointer to it
		 */
        Instruction * valueInstruction;
        Value * valueIntoPtrStore;
        Value * ptrToValueCast;

        /*
         * Queue push of value
         */
		CallInst * pushQueueCall;
	};
	
	struct StageInfo {
		SCC *scc;
		Function *sccStage;
		BasicBlock *entryBlock, *exitBlock;

		/*
		 * Maps instructions from original instructions in the function to the stages' clones 
		 */
		unordered_map<Instruction *, Instruction *> * iCloneMap;
		unordered_map<BasicBlock*, BasicBlock*> * bbCloneMap;

		/*
		 * Stores incoming/outgoing edges from other strongly connected components
		 */
		std::vector<DGEdge<Value> *> incomingSCCEdges, outgoingSCCEdges;

		/*
		 * Maps internal instructions to external values for incoming/outgoing loop dependencies
		 */
        unordered_map<Instruction *, Value *> incomingDependentMap, outgoingDependentMap;

        /*
         * Maps external dependency to its location in the environment used by the stage handler
         */
        unordered_map<Value *, int> externalDependencyToEnvMap;

        /*
         * Maps internal dependency to its queue
         */
        unordered_map<DGEdge<Value> *, int> edgeToQueueMap;

        std::vector<std::unique_ptr<OutgoingPipelineInfo>> valuePushQueues;
        std::map<Instruction *, std::unique_ptr<IncomingPipelineInfo>> valuePopQueuesMap;
	};
}