#pragma once

#include "llvm/IR/Instructions.h"
#include "PDG.hpp"
#include "SCC.hpp"

#include <unordered_map>

using namespace std;
using namespace llvm;

namespace llvm {

	struct IncomingPipelineInfo {
		CallInst * popQueueCall;
        Instruction * popStorage;
        LoadInst * loadStorage;

        std::vector<Instruction *> userInstructions;
	};

	struct OutgoingPipelineInfo {
		CallInst * pushQueueCall;

        Instruction * valueInstruction;
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
		std::vector<DGEdge<Instruction> *> incomingSCCEdges, outgoingSCCEdges;

		/*
		 * Maps internal, external pair of instructions for incoming/outgoing loop dependencies
		 */
        unordered_map<Instruction *, Instruction *> incomingDependentMap, outgoingDependentMap;

        /*
         * Maps external dependency to its location in the environment used by the stage handler
         */
        unordered_map<Instruction *, int> externalDependencyToEnvMap;

        /*
         * Maps internal dependency to its queue
         */
        unordered_map<DGEdge<Instruction> *, int> edgeToQueueMap;

        std::vector<std::unique_ptr<OutgoingPipelineInfo>> valuePushQueues;
        std::map<Instruction *, std::unique_ptr<IncomingPipelineInfo>> valuePopQueuesMap;
	};
}