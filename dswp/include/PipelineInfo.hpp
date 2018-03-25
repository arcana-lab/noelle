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
        unordered_map<Instruction *, int> dependencyToEnvironmentMap;

        std::vector<OutgoingPipelineInfo *> valuePushQueues;
        std::map<Instruction *, IncomingPipelineInfo *> valuePopQueuesMap;
	};
}