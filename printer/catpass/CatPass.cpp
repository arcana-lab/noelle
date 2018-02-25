#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/LoopInfo.h"

#include "DGBase.hpp"
#include "DGGraphTraits.hpp"
#include "PDG.hpp"
#include "PDGAnalysis.hpp"
#include "SCCDG.hpp"

#include "llvm/ADT/GraphTraits.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"
#include "llvm/Analysis/DomPrinter.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/DOTGraphTraits.h"

using namespace llvm;

namespace llvm {
  struct PDGPrinter : public ModulePass {
    static char ID;

    PDGPrinter() : ModulePass{ID} {}

    bool doInitialization (Module &M) override {
      errs() << "PDGPrinter at \"doInitialization\"\n" ;
      return false;
    }

    template <class GT>
    void writeGraph(const std::string& filename, GT *graph) {
      errs() << "Writing '" << filename << "'...\n";

      std::error_code EC;
      raw_fd_ostream File(filename, EC, sys::fs::F_Text);
      std::string Title = DOTGraphTraits<GT *>::getGraphName(graph);

      if (!EC) {
        WriteGraph(File, graph, false, Title);
        errs() << "\n";
      } else {
        errs() << "  error opening file for writing!\n";
        abort();
      }
    }

    bool runOnModule (Module &M) override {
      errs() << "PDGPrinter at \"runOnModule\"\n";

      auto *graph = getAnalysis<PDGAnalysis>().getPDG();
      
      writeGraph<PDG>("pdg-full.dot",graph);

      for (auto &F : M) {
        if (F.empty()) continue ;
        std::string filename;
        raw_string_ostream ros(filename);
        ros << "pdg-" << F.getName() << ".dot";

        auto *subgraph = graph->createFunctionSubgraph(F);
        writeGraph<PDG>(ros.str(), subgraph);

        filename.clear();
        ros << "sccdg-" << F.getName() << ".dot";
        SCCDG *sccSubgraph = SCCDG::createSCCGraphFrom(subgraph);
        writeGraph<SCCDG>(ros.str(), sccSubgraph);

        int count = 0;
        for (auto sccI = sccSubgraph->begin_nodes(); sccI != sccSubgraph->end_nodes(); ++sccI) {
          auto scc = (*sccI)->getNode();
          filename.clear();
          ros << "scc-" << F.getName() << "-" << (count++) << ".dot";
          writeGraph<SCC>(ros.str(), scc);
        }

        delete sccSubgraph;
        delete subgraph;

        LoopInfo& LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
        if (LI.empty()) continue ;
        filename.clear();
        ros << "pdg-" << F.getName() << "-loops.dot";

        subgraph = graph->createLoopsSubgraph(LI);
        writeGraph<PDG>(ros.str(), subgraph);
        delete subgraph;
      }

      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<LoopInfoWrapperPass>();
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
