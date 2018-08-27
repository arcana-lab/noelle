#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "llvm/Analysis/LoopInfo.h"

#include "LoopDependenceInfo.hpp"

using namespace std;
using namespace llvm;

llvm::LoopDependenceInfo::LoopDependenceInfo(Function *f, PDG *fG, Loop *l, LoopInfo &li, PostDominatorTree &pdt)
		: function{f}, functionDG{fG} {

    /*
     * Create a LoopInfo summary
     */
    this->liSummary.populate(li, l);

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
      loopBBtoPD[&*bb] = pdt.getNode(&*bb)->getIDom()->getBlock();
    }

    /*
     * Set the loop dependence graph.
     */
	this->loopDG = functionDG->createLoopsSubgraph(l);

	/*
	 * Build a SCCDAG of loop-internal instructions
	 */
	std::vector<Value *> loopInternals;
	for (auto internalNode : loopDG->internalNodePairs()) {
		loopInternals.push_back(internalNode.first);
	}
	loopInternalDG = loopDG->createSubgraphFromValues(loopInternals, false);
	loopSCCDAG = SCCDAG::createSCCDAGFrom(loopInternalDG);

	l->getExitBlocks(loopExitBlocks);

  environment = new LoopEnvironment(this->loopDG, this->loopExitBlocks);

  return ;
};

void LoopDependenceInfo::createPDGs (void){
  return ;
}

llvm::LoopDependenceInfo::~LoopDependenceInfo() {
  delete loopDG;
  delete loopInternalDG;
  delete loopSCCDAG;
  delete environment;

  return ;
}
