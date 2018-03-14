#pragma once

#include "llvm/IR/Instructions.h"

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
}