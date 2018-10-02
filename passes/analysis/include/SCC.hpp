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
        enum SCCType {SEQUENTIAL, COMMUTATIVE, INDEPENDENT};

        SCC(std::set<DGNode<Value> *> nodes, bool connectToExternalValues = true) ;

        raw_ostream &print (raw_ostream &stream, std::string prefixToUse = "", int maxEdges = 15) ;
        raw_ostream &printMinimal (raw_ostream &stream, std::string prefixToUse = "") ;

        bool hasCycle (bool ignoreControlDep = false) ;

        SCCType getType (void) const;

        void setType (SCCType t);

        ~SCC() ;

      private:
        SCCType sccType;
	};

	template<> class DGEdge<SCC> : public DGEdgeBase<SCC, Value> {
	public:
		DGEdge(DGNode<SCC> *src, DGNode<SCC> *dst) : DGEdgeBase<SCC, Value>(src, dst) {}
		DGEdge(const DGEdge<SCC> &oldEdge) : DGEdgeBase<SCC, Value>(oldEdge) {}
	};
}
