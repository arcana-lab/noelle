#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <algorithm>

#include "noelle/core/Noelle.hpp"

using namespace arcana::noelle;

namespace {

struct CAT : public ModulePass {
  static char ID;

  CAT() : ModulePass(ID) {}

  bool doInitialization(Module &M) override {
    return false;
  }

  bool runOnModule(Module &M) override {
    errs() << "SCCCAG: Start\n";
    errs() << "\n";

    /*
     * Fetch NOELLE
     */
    auto &noelle = getAnalysis<Noelle>();

    /*
     * Fetch the entry point.
     */
    auto fm = noelle.getFunctionsManager();

    /*
     * Fetch the SCCDAG of the program call graph: SCCCAG
     */
    auto sccCAG = fm->getSCCDAGOfProgramCallGraph();

    /*
     * Print the nodes of the SCCCAG.
     */
    errs() << "SCCCAG:   Nodes\n";
    for (auto node : sccCAG->getNodes()) {

      /*
       * Print the node.
       */
      errs() << "SCCCAG:     " << node->getID() << ": ";
      if (node->isAnSCC()) {
        auto sccNode = static_cast<SCCCAGNode_SCC *>(node);
        errs() << "SCC\n";
        errs() << "SCCCAG:       Internal nodes:\n";
        for (auto internalNode : sccNode->getInternalNodes()) {
          auto f = internalNode->getFunction();
          errs() << "SCCCAG:         " << f->getName() << "\n";
        }

      } else {
        auto fNode = static_cast<SCCCAGNode_Function *>(node);
        errs() << "Function " << fNode->getNode()->getFunction()->getName()
               << "\n";
      }
    }
    errs() << "\n";

    /*
     * Print the outgoing edges.
     */
    errs() << "SCCCAG:   Edges\n";
    for (auto node : sccCAG->getNodes()) {
      for (auto dstNodePair : sccCAG->getOutgoingEdges(node)) {
        auto edge = dstNodePair.second;
        auto dstNode = edge->getDst();
        errs() << "SCCCAG:     " << node->getID() << " -> " << dstNode->getID()
               << "\n";

        /*
         * Print the sub-edges.
         */
        errs()
            << "SCCCAG:       Because of the following edges in the call graph:\n";
        for (auto subEdge : edge->getSubEdges()) {
          auto callerNode = subEdge->getCaller();
          auto calleeNode = subEdge->getCallee();
          auto calleer = callerNode->getFunction();
          auto callee = calleeNode->getFunction();
          errs() << "SCCCAG:         \"" << calleer->getName()
                 << "\" invokes \"" << callee->getName() << "\"\n";
        }
      }
    }
    errs() << "\n";

    /*
     * Print all nodes that have no incoming edges.
     */
    errs() << "SCCCAG:   Nodes without incoming edges\n";
    for (auto node : sccCAG->getNodesWithInDegree(0)) {
      errs() << "SCCCAG:     " << node->getID() << "\n";
    }
    errs() << "\n";

    /*
     * Print all nodes that have no outgoing edges.
     */
    errs() << "SCCCAG:   Nodes without outgoing edges\n";
    for (auto node : sccCAG->getNodesWithOutDegree(0)) {
      errs() << "SCCCAG:     " << node->getID() << "\n";
    }
    errs() << "\n";

    errs() << "SCCCAG: End\n";
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<Noelle>();
  }
};
} // namespace

// Next there is code to register your pass to "opt"
char CAT::ID = 0;
static RegisterPass<CAT> X("CAT", "Simple user of the Noelle framework");

// Next there is code to register your pass to "clang"
static CAT *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
                                        [](const PassManagerBuilder &,
                                           legacy::PassManagerBase &PM) {
                                          if (!_PassMaker) {
                                            PM.add(_PassMaker = new CAT());
                                          }
                                        }); // ** for -Ox
static RegisterStandardPasses _RegPass2(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new CAT());
      }
    }); // ** for -O0
