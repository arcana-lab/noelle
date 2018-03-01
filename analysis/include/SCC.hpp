#pragma once

#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "DGBase.hpp"

using namespace llvm;

namespace llvm {

	/*
	* Strongly Connected Component
	*/
	class SCC : public DG<Instruction> {
	public:
		SCC(std::vector<DGNode<Instruction> *> nodes) ;
		~SCC() ;

		raw_ostream &print(raw_ostream &stream) ;
	};
}