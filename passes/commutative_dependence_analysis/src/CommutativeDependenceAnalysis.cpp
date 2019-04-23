
#include "UniqueIRMarkerPass.hpp"

#include <type_traits>


#include "CommutativeDependenceAnalysis.hpp"

using std::addressof;

CommutativeDependenceAnalysisPass::CommutativeDependenceAnalysisPass() : ModulePass(ID) {}

void CommutativeDependenceAnalysisPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<UniqueIRMarkerPass>();
  AU.setPreservesAll();
}

bool CommutativeDependenceAnalysisPass::doInitialization(Module &M) {
  CommDepSource = std::make_unique<CommutativeDependenceSource>(M);
}

bool CommutativeDependenceAnalysisPass::runOnModule(Module &M) {
  CommDepSource->loadCommutativeDependencies();

  return false;
}

iterator_range<CommutativeDependenceSource::comm_dep_iterator>
CommutativeDependenceAnalysisPass::getCommutativeDependencies(Function *F) {
  return CommDepSource->getCommutativeEdges(F);
}

CommutativeDependenceSource *CommutativeDependenceAnalysisPass::getCommutativeDependenceSource() {
  return CommDepSource.get();
}

char CommutativeDependenceAnalysisPass::ID = 0;
static RegisterPass<CommutativeDependenceAnalysisPass> X("CommutativeDependenceAnalysis",
    "Finds and removes commutative dependencies from the PDG, if this pass is run CommSync must "
    "be run after. Usually after a parallelisation technique has been run");
