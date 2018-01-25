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
#include "llvm/Support/DOTGraphTraits.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"
#include "llvm/Analysis/PostDominators.h"

#include "PDGAnalysis.hpp"
#include "PDG.hpp"

using namespace llvm;

namespace llvm {

  template<> struct DOTGraphTraits<PDGNode*> : public DefaultDOTGraphTraits {
    explicit DOTGraphTraits(bool isSimple=false) : DefaultDOTGraphTraits(isSimple) {}
    
  };

  template<>
  struct DOTGraphTraits<PDG*> : public DOTGraphTraits<PDGNode*> {
    DOTGraphTraits (bool isSimple=false) : DOTGraphTraits<PDGNode*>(isSimple) {}

    static std::string getGraphName(PDG *pdg) {
      return "PDG tree";
    }

    std::string getNodeLabel(PDGNode *node, PDG *pdg) {
      return DOTGraphTraits<PDGNode*>::getNodeLabel(node, pdg->getRootNode());
    }
  };

  struct PDGPrinter : public ModulePass {
    static char ID;

    PDGPrinter() : ModulePass{ID} {}

    bool doInitialization (Module &M) override {
      errs() << "PDGPrinter at \"doInitialization\"\n" ;
      return false;
    }

    bool runOnModule (Module &M) override {
      errs() << "PDGPrinter at \"runOnModule\"\n";
      return false;

      PDG *Graph = new PDG(M);
      std::string Filename = "pdg.dot";
      std::error_code EC;

      errs() << "Writing '" << Filename << "'...";

      raw_fd_ostream File(Filename, EC, sys::fs::F_Text);
      std::string Title = DOTGraphTraits<PDG>::getGraphName(Graph);

      if (!EC)
        WriteGraph(File, Graph, false, Title);
      else
        errs() << "  error opening file for writing!";
      errs() << "\n";

      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<PDGAnalysis>();
      AU.setPreservesAll();

      return ;
    }
  };
}

// Next there is code to register your pass to "opt"
char llvm::PDGPrinter::ID = 0;
static RegisterPass<PDGPrinter> X("PDGPrinter", "Program Dependence Graph .dot file printer");

// Next there is code to register your pass to "clang"
static PDGPrinter * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGPrinter());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new PDGPrinter());}});// ** for -O0
