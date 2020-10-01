// TODO: add copyright

#include "LoopAwareMemDepAnalysis.hpp"
#include "DataFlow.hpp"

#include "Utilities/PDGQueries.h"

using namespace llvm;

void llvm::refinePDGWithLoopAwareMemDepAnalysis(
  PDG *loopDG,
  Loop *l,
  LoopStructure *loopStructure,
  LoopCarriedDependencies &LCD,
  liberty::LoopAA *loopAA,
  LoopIterationDomainSpaceAnalysis *LIDS
) {

  // TODO: add here other types of loopAware refinements of the PDG

  if (loopAA) {
    refinePDGWithSCAF(loopDG, l, loopAA);
  }

  if (LIDS) {
    refinePDGWithLIDS(loopDG, loopStructure, LCD, LIDS);
  }

}

void llvm::refinePDGWithSCAF(PDG *loopDG, Loop *l, liberty::LoopAA *loopAA) {
  // Iterate over all the edges of the loop PDG and
  // collect memory deps to be queried.
  // For each pair of instructions with a memory dependence map it to
  // a small vector of found edges (0th element is for RAW, 1st for WAW, 2nd for WAR)
  map<pair<Instruction *, Instruction *>, SmallVector<DGEdge<Value> *, 3>>
      memDeps;
  for (auto edge : make_range(loopDG->begin_edges(), loopDG->end_edges())) {
    if (!loopDG->isInternal(edge->getIncomingT()) ||
        !loopDG->isInternal(edge->getOutgoingT()))
      continue;

    if (!edge->isMemoryDependence())
      continue;

		Value *pdgValueI = edge->getOutgoingT();
    Instruction *i = dyn_cast<Instruction>(pdgValueI);
    assert(i && "Expecting an instruction as the value of a PDG node");

    Value *pdgValueJ = edge->getIncomingT();
    Instruction *j = dyn_cast<Instruction>(pdgValueJ);
    assert(j && "Expecting an instruction as the value of a PDG node");

		if (!memDeps.count({i,j})) {
			memDeps[{i,j}] = {nullptr, nullptr, nullptr};
		}

		if (edge->isRAWDependence()) {
			memDeps[{i,j}][0] = edge;
		}
		else if (edge->isWAWDependence()) {
			memDeps[{i,j}][1] = edge;
		}
		else if (edge->isWARDependence()) {
			memDeps[{i,j}][2] = edge;
		}
  }

  // For each memory depedence perform loop-aware dependence analysis to
  // disprove it. Queries for loop-carried and intra-iteration deps.
  for (auto memDep : memDeps) {
    auto instPair = memDep.first;
    Instruction *i = instPair.first;
    Instruction *j = instPair.second;
		auto edges = memDep.second;

		// encode the found dependences in a bit vector.
		// set least significant bit for RAW, 2nd bit for WAW, 3rd bit for WAR
    uint8_t depTypes = 0;
		for (uint8_t i = 0; i <= 2; ++i) {
			if (edges[i]) {
				depTypes |= 1 << i;
			}
		}
		// Try to disprove all the reported loop-carried deps
    uint8_t disprovedLCDepTypes =
        disproveLoopCarriedMemoryDep(i, j, depTypes, l, loopAA);
		// set LoopCarried bit for all the non-disproved LC edges
		uint8_t lcDepTypes = depTypes - disprovedLCDepTypes;
		for (uint8_t i = 0; i <= 2; ++i) {
			if (lcDepTypes & (1 << i)) {
				auto &e = edges[i];
				e->setLoopCarried(true);
			}
		}

		// for every disproved loop-carried dependence
    // check if there is a intra-iteration dependence
    uint8_t disprovedIIDepTypes = 0;
    if (disprovedLCDepTypes) {
      disprovedIIDepTypes = disproveIntraIterationMemoryDep(
          i, j, disprovedLCDepTypes, l, loopAA);

      // remove any edge that SCAF disproved both its loop-carried and
      // intra-iteration version
      for (uint8_t i = 0; i <= 2; ++i) {
        if (disprovedIIDepTypes & (1 << i)) {
          auto &e = edges[i];
          loopDG->removeEdge(e);
        }
			}

			// set LoopCarried bit false for all the non-disproved intra-iteration edges
      // (but were not loop-carried)
			uint8_t iiDepTypes = disprovedLCDepTypes - disprovedIIDepTypes;
			for (uint8_t i = 0; i <= 2; ++i) {
				if (iiDepTypes & (1 << i)) {
					auto &e = edges[i];
					e->setLoopCarried(false);
				}
			}
		}
	}
}

// TODO: Refactor along with HELIX's exact same implementation of this method
DataFlowResult *computeReachabilityFromInstructions (LoopStructure *loopStructure) {

  auto loopHeader = loopStructure->getHeader();
  auto loopFunction = loopStructure->getFunction();

  /*
   * Run the data flow analysis needed to identify the locations where signal instructions will be placed.
   */
  auto dfa = DataFlowEngine{};
  auto computeGEN = [](Instruction *i, DataFlowResult *df) {
    auto& gen = df->GEN(i);
    gen.insert(i);
    return ;
  };
  auto computeKILL = [](Instruction *, DataFlowResult *) {
    return ;
  };
  auto computeOUT = [loopHeader](std::set<Value *>& OUT, Instruction *succ, DataFlowResult *df) {

    /*
     * Check if the successor is the header.
     * In this case, we do not propagate the reachable instructions.
     * We do this because we are interested in understanding the reachability of instructions within a single iteration.
     */
    auto succBB = succ->getParent();
    if (succ == &*loopHeader->begin()) {
      return ;
    }

    /*
     * Propagate the data flow values.
     */
    auto& inS = df->IN(succ);
    OUT.insert(inS.begin(), inS.end());
    return ;
  } ;
  auto computeIN = [](std::set<Value *>& IN, Instruction *inst, DataFlowResult *df) {
    auto& genI = df->GEN(inst);
    auto& outI = df->OUT(inst);
    IN.insert(outI.begin(), outI.end());
    IN.insert(genI.begin(), genI.end());
    return ;
  };

  return dfa.applyBackward(loopFunction, computeGEN, computeKILL, computeIN, computeOUT);
}

void llvm::refinePDGWithLIDS(
  PDG *loopDG,
  LoopStructure *loopStructure,
  LoopCarriedDependencies &LCD,
  LoopIterationDomainSpaceAnalysis *LIDS
) {

  auto dfr = computeReachabilityFromInstructions(loopStructure);

  std::unordered_set<DGEdge<Value> *> edgesThatExist;
  for (auto edge : loopDG->getEdges()) {
    edgesThatExist.insert(edge);
  }

  std::unordered_set<DGEdge<Value> *> edgesToRemove;
  for (auto dependency : LCD.getLoopCarriedDependenciesForLoop(*loopStructure)) {

    /*
     * The edge could have already been removed by another refining step
     * Check that the edge still exists
     */
    if (edgesThatExist.find(dependency) == edgesThatExist.end()) continue;

    /*
     * Do not waste time on edges that aren't memory dependencies
     */
    if (!dependency->isMemoryDependence()) continue;

    auto fromInst = dyn_cast<Instruction>(dependency->getOutgoingT());
    auto toInst = dyn_cast<Instruction>(dependency->getIncomingT());
    if (!fromInst || !toInst) continue;

    /*
     * Loop carried dependencies are conservatively marked as such; we can only
     * remove dependencies between a producer and consumer where we know the producer
     * can NEVER reach the consumer during the same iteration
     */
    auto &afterInstructions = dfr->OUT(fromInst);
    if (afterInstructions.find(toInst) != afterInstructions.end()) continue;

    if (LIDS->areInstructionsAccessingDisjointMemoryLocationsBetweenIterations(fromInst, toInst)) {
      edgesToRemove.insert(dependency);
    }
  }

  for (auto edge : edgesToRemove) {
    loopDG->removeEdge(edge);
  }

}
