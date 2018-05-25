#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"

#include "LoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

llvm::LoopDependenceInfo::LoopDependenceInfo(Function *f, PDG *fG, Loop *l, LoopInfo &li)
		: function{f}, functionDG{fG} {

    /*
     * Set the headers.
     */
    this->header = l->getHeader();
    this->preHeader = l->getLoopPreheader();

    /*
     * Set the loop body.
     */
    for (auto bb : l->blocks()){
      this->loopBBs.push_back(&*bb);
    }

    /*
     * Set the loop dependence graph.
     */
	this->loopDG = functionDG->createLoopsSubgraph(li);

	/*
	 * Build a SCCDAG of loop-internal instructions
	 */
	std::vector<Value *> loopInternals;
	for (auto internalNode : loopDG->internalNodePairs()) {
		loopInternals.push_back(internalNode.first);
		// internalNode.first->print(errs()); errs() << "\n";
	}
	loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
	loopSCCDAG = SCCDAG::createSCCDAGFrom(loopInternalDG);

	l->getExitBlocks(loopExitBlocks);
    return ;
};

llvm::LoopDependenceInfo::~LoopDependenceInfo() {
	delete loopDG;
	delete loopInternalDG;
	delete loopSCCDAG;

    return ;
}
