#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

//#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Dominators.h"
//#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Analysis/DomPrinter.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"
#include "llvm/Analysis/PostDominators.h"

using namespace llvm;

namespace llvm {
  template<>
  struct DOTGraphTraits<DominatorTree*> : public DOTGraphTraits<DomTreeNode*> {

    DOTGraphTraits (bool isSimple=false)
      : DOTGraphTraits<DomTreeNode*>(isSimple) {}

    static std::string getGraphName(DominatorTree *DT) {
      return "PDG tree";
    }

    std::string getNodeLabel(DomTreeNode *Node, DominatorTree *G) {
      return DOTGraphTraits<DomTreeNode*>::getNodeLabel(Node, G->getRootNode());
    }
  };

  struct DominatorTreeWrapperPassAnalysisGraphTraits {
    static DominatorTree *getGraph(DominatorTreeWrapperPass *DTWP) {
      return &DTWP->getDomTree();
    }
  };

  struct PDGPrinter : public DOTGraphTraitsPrinter<DominatorTreeWrapperPass, true, DominatorTree*, DominatorTreeWrapperPassAnalysisGraphTraits> {
    static char ID;

    PDGPrinter() : DOTGraphTraitsPrinter<DominatorTreeWrapperPass, true, DominatorTree*, DominatorTreeWrapperPassAnalysisGraphTraits> ("pdg", ID) {}

    bool doInitialization (Module &M) override {
      errs() << "PDGPrinter at \"doInitialization\"\n" ;
      return false;
    }

    bool runOnFunction (Function &F) override {
      errs() << "PDGPrinter at \"runOnFunction\"\n" ;
      DominatorTree domTree = DominatorTree(F);
      for (auto &B : F) {
        TerminatorInst *I = B.getTerminator();
        for (auto i = 0; i < I->getNumSuccessors(); ++i) {
          errs() << domTree.dominates(I, I->getSuccessor(i)) << "\n";
        }
      }
      return DOTGraphTraitsPrinter<DominatorTreeWrapperPass, true, DominatorTree*, DominatorTreeWrapperPassAnalysisGraphTraits>::runOnFunction(F);
    }
  };
}

//char PDG::ID = 0;
//static RegisterPass<PDG> X("PDG", "Program Dependence Calculator");

// Next there is code to register your pass to "opt"
char PDGPrinter::ID = 0;
static RegisterPass<PDGPrinter> X("PDGPrinter", "Program Dependence Graph .dot file printer");

// Next there is code to register your pass to "clang"
static PDGPrinter * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGPrinter());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGPrinter());}});// ** for -O0
