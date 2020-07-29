// TODO: add copyright

#include "LoopAwareMemDepAnalysis.hpp"

#include "Utilities/PDGQueries.hpp"

using namespace llvm;

void refinePDGWithLoopAwareMemDepAnalysis(PDG *loopDG, Loop *l,
                                          liberty::LoopAA *loopAA) {
  // TODO: add here other types of loopAware refinements of the PDG

  refinePDGWithSCAF(loopDG, l, loopAA);
}

void refinePDGWithSCAF(PDG *loopDG, Loop *l, liberty::LoopAA *loopAA) {
  // Iterate over all the edges of the loop PDG and
  // collect memory deps to be queried.
  // For each pair of instructions with a memory dependence map it to
  // a tuple of found edges (0th element is for RAW, 1st for WAW, 2nd for WAR)
	typedef std::tuple<DGEdge<Value>*,DGEdge<Value>*, DGEdge<Value>*> DepTuple;
  map<pair<Instruction*, Instruction*>, DepTuple> memDeps;
	for (auto edge : make_range(loopDG.begin_edges(), loopDG.end_edges())) {
    if (!loopDG.isInternal(edge->getIncomingT()) ||
        !loopDG.isInternal(edge->getOutgoingT()))
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
			get<0>(memDeps[{i,j}]) = edge;
		}
		else if (edge->isWAWDependence()) {
			get<1>(memDeps[{i,j}]) = edge;
		}
		else if (edge->isWARDependence()) {
			get<2>(memDeps[{i,j}]) = edge;
		}
  }

  // For each memory depedence perform loop-aware dependence analysis to
  // disprove it. Queries for loop-carried and intra-iteration deps.
  for (auto memDep : memDeps) {
    auto pairI = memDep.first;
    Instruction *i = pairI.first;
    Instruction *j = pairJ.second;
		auto edges = memDep.second;

		// encode the found dependences in a bit vector.
		// set least significant bit for RAW, 2nd bit for WAW, 3rd bit for WAR
    uint8_t depTypes = 0;
		for (uint8_t i = 0; i <= 2; ++i) {
			if (get<i>(edges)) {
				depTypes |= 1 << i;
			}
		}
		// Try to disprove all the reported loop-carried deps
    uint8_t disprovedLCDepTypes =
        disproveLoopCarriedMemoryDep(i, j, depTypes, loop, loopAA);
		// set LoopCarried bit for all the non-disproved LC edges
		uint8_t lcDepTypes = depTypes - disprovedLCDepTypes;
		for (uint8_t i = 0; i <= 2; ++i) {
			if (lcDepTypes & (1 << i)) {
				auto &e = get<i>(edges);
				e->setLoopCarried(true);
			}
		}

		// for every disproved loop-carried dependence
    // check if there is a intra-iteration dependence
    uint8_t disprovedIIDepTypes = 0;
    if (disprovedLCDepTypes) {
      disprovedIIDepTypes = disproveIntraIterationMemoryDep(
          i, j, disprovedLCDepTypes, loop, loopAA);

      // remove any edge that SCAF disproved both its loop-carried and
      // intra-iteration version
      for (uint8_t i = 0; i <= 2; ++i) {
        if (disprovedIIDepTypes & (1 << i)) {
          auto &e = get<i>(edges);
          loopDG->removeEdge(e);
        }
			}

			// set LoopCarried bit false for all the non-disproved intra-iteration edges
      // (but were not loop-carried)
			uint8_t iiDepTypes = disprovedLCDepTypes - disprovedIIDepTypes;
			for (uint8_t i = 0; i <= 2; ++i) {
				if (iiDepTypes & (1 << i)) {
					auto &e = get<i>(edges);
					e->setLoopCarried(false);
				}
			}
		}
	}
}
