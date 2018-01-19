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
/*
  template <typename AnalysisT, bool IsSimple, typename GraphT = AnalysisT *,
            typename AnalysisGraphTraitsT = DefaultAnalysisGraphTraits<AnalysisT> >
  class DOTPrinter : public FunctionPass {
  public:
    DOTPrinter(StringRef GraphName, char &ID)
        : FunctionPass(ID), Name(GraphName) {}

    /// @brief Return true if this function should be processed.
    ///
    /// An implementation of this class my override this function to indicate that
    /// only certain functions should be printed.
    ///
    /// @param Analysis The current analysis result for this function.
    virtual bool processFunction(Function &F, AnalysisT &Analysis) {
      return true;
    }

    bool runOnFunction(Function &F) override {
      auto &Analysis = getAnalysis<AnalysisT>();

      if (!processFunction(F, Analysis))
        return false;

      GraphT Graph = AnalysisGraphTraitsT::getGraph(&Analysis);
      std::string Filename = Name + "." + F.getName().str() + ".dot";
      std::error_code EC;

      errs() << "Writing '" << Filename << "'...";

      raw_fd_ostream File(Filename, EC, sys::fs::F_Text);
      std::string GraphName = DOTGraphTraits<GraphT>::getGraphName(Graph);
      std::string Title = GraphName + " for '" + F.getName().str() + "' function";

      if (!EC)
        WriteGraph(File, Graph, IsSimple, Title);
      else
        errs() << "  error opening file for writing!";
      errs() << "\n";

      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
      AU.addRequired<AnalysisT>();
    }

  private:
    std::string Name;
  };
*/
  /*

  class PDTreeWrapperPass : public FunctionPass {
    PDTree DT;
    static char ID;

    PDTreeWrapperPass() : FunctionPass(ID) {
    }

    PDTree &getDomTree() { return DT; }
    const PDTree &getDomTree() const { return DT; }

    bool runOnFunction(Function &F) override;

    void verifyAnalysis() const override;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }

    void releaseMemory() override { DT.releaseMemory(); }

    void print(raw_ostream &OS, const Module *M = nullptr) const override;
  };

  // Needs to extend some TreeBase class so it behaves the way GraphWriter wants (see llvm/Support/GraphWriter.h)
  class PDTree {
  };

  struct PDTreeWrapperPassAnalysisGraphTraits {
    static PDTree *getGraph(PDTreeWrapperPass *PDWP) {
      return new PDTree();
      //return &PDWP->getDomTree();
    }
  };
  
  template<>
  struct DOTGraphTraits<PDTree*> : public DOTGraphTraits<DominatorTree*> {

    DOTGraphTraits (bool isSimple=false)
      : DOTGraphTraits<DominatorTree*>(isSimple) {}

    static std::string getGraphName(PDTree *PD) {
      return "Program Dependence tree";
    }
  };

  */

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
      return DOTPrinter<DominatorTreeWrapperPass, true, DominatorTree*, DominatorTreeWrapperPassAnalysisGraphTraits>::runOnFunction(F);
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
