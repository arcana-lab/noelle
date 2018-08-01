#pragma once

#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "DGBase.hpp"

using namespace llvm;

namespace llvm {

	/*
	* Strongly Connected Component
	*/
	class SCC : public DG<Value> {
	public:
		SCC(std::set<DGNode<Value> *> nodes) ;
		~SCC() ;

		raw_ostream &print(raw_ostream &stream, std::string prefixToUse = "") ;
		bool hasCycle (bool ignoreControlDep = false) ;
	};

	template<> class DGEdge<SCC> : public DGEdgeBase<SCC, Value> {
	public:
		DGEdge(DGNode<SCC> *src, DGNode<SCC> *dst) : DGEdgeBase<SCC, Value>(src, dst) {}
		DGEdge(const DGEdge<SCC> &oldEdge) : DGEdgeBase<SCC, Value>(oldEdge) {}
	};
}
