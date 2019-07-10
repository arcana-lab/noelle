/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
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
        enum SCCType {SEQUENTIAL, REDUCIBLE, INDEPENDENT};

        SCC (std::set<DGNode<Value> *> nodes, bool connectToExternalValues = true) ;

        bool iterateOverInstructions (std::function<bool (Instruction *)> funcToInvoke);

        bool hasCycle (bool ignoreControlDep = false) ;

        SCCType getType (void) const;

        void setType (SCCType t);

        int64_t numberOfInstructions (void) const ;

        raw_ostream &print (raw_ostream &stream, std::string prefixToUse = "", int maxEdges = 15) ;

        raw_ostream &printMinimal (raw_ostream &stream, std::string prefixToUse = "") ;

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
