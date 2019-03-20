
#include "UniqueIRMarker.hpp"
#include "PDGAnalysis.hpp"
#include "SCCDAG.hpp"

#include "CommutativeDependenceAnalysis.hpp"


CommutativeDependenceAnalysisPass::CommutativeDependenceAnalysisPass() : ModulePass(ID) {}

void CommutativeDependenceAnalysisPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<UniqueIRMarkerPass>();
  AU.addRequired<PDGAnalysis>();
  AU.setPreservesAll();
}

bool CommutativeDependenceAnalysisPass::doInitialization(Module &M) {
  CommDepSource = std::make_unique<CommutativeDependenceSource>(M);
}

bool CommutativeDependenceAnalysisPass::runOnModule(Module &M) {
  CommDepSource->loadCommutativeDependencies();
  auto *PDG = getAnalysis<PDGAnalysis>().getPDG();

  return false;
}

iterator_range<CommutativeDependenceSource::comm_dep_iterator>
CommutativeDependenceAnalysisPass::getRemovedDependencies(Function *F) {
  return CommDepSource->getCommutativeEdges(F);
}

char CommutativeDependenceAnalysisPass::ID = 0;
static RegisterPass<CommutativeDependenceAnalysisPass> X("CommutativeDependenceAnalysis",
    "Finds and removes commutative dependencies from the PDG, if this pass is run CommSync must "
    "be run after. Usually after a parallelisation technique has been run");
