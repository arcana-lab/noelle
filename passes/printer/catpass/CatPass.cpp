/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CallGraph.h"

#include "DGBase.hpp"
#include "DGGraphTraits.hpp"
#include "PDG.hpp"
#include "PDGAnalysis.hpp"
#include "SCCDAG.hpp"

#include "llvm/ADT/GraphTraits.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"
#include "llvm/Analysis/DomPrinter.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/DOTGraphTraits.h"

#include <set>
#include <queue>

using namespace llvm;

namespace llvm {
  struct PDGPrinter : public ModulePass {
  public:
    static char ID;

    PDGPrinter() : ModulePass{ID} {}

    bool doInitialization (Module &M) override {
      errs() << "PDGPrinter at \"doInitialization\"\n" ;
      return false;
    }

    bool runOnModule (Module &M) override {
      errs() << "PDGPrinter at \"runOnModule\"\n";

      /*
       * Collect functions through call graph starting at function "main"
       */
      std::set<Function *> funcToGraph;
      collectAllFunctionsInCallGraph(M, funcToGraph);

      auto *graph = getAnalysis<PDGAnalysis>().getPDG();
      writeGraph<PDG>("pdg-full.dot", graph);
      for (auto F : funcToGraph) {
        printGraphsForFunction(*F, graph);
      }

      return false;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addRequired<CallGraphWrapperPass>();
      AU.addRequired<PDGAnalysis>();
      AU.setPreservesAll();
      return ;
    }

  private:
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

    void collectAllFunctionsInCallGraph (Module &M, std::set<Function *> &funcSet)
    {
      auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
      std::queue<Function *> funcToTraverse;
      funcToTraverse.push(M.getFunction("main"));
      while (!funcToTraverse.empty())
      {
        auto func = funcToTraverse.front();
        funcToTraverse.pop();
        if (funcSet.find(func) != funcSet.end()) continue;
        funcSet.insert(func);

        auto funcCGNode = callGraph[func];
        for (auto &callRecord : make_range(funcCGNode->begin(), funcCGNode->end()))
        {
          auto F = callRecord.second->getFunction();
          if (F->empty()) continue;
          funcToTraverse.push(F);
        }
      }
    }

    void printGraphsForFunction(Function &F, PDG *graph)
    {
      /*
       * Name and graph the function's DG
       */
      std::string filename;
      raw_string_ostream ros(filename);
      ros << "pdg-" << F.getName() << ".dot";
      auto *subgraph = graph->createFunctionSubgraph(F);
      writeGraph<PDG>(ros.str(), subgraph);

      /*
       * Name and graph the function's SCCDAG
       */
      filename.clear();
      ros << "sccdg-" << F.getName() << ".dot";
      SCCDAG *sccSubgraph = SCCDAG::createSCCDAGFrom(subgraph);
      writeGraph<SCCDAG>(ros.str(), sccSubgraph);

      /*
       * Name and graph each SCC within the function's SCCDAG
       */
      int count = 0;
      for (auto sccI = sccSubgraph->begin_nodes(); sccI != sccSubgraph->end_nodes(); ++sccI) {
        auto scc = (*sccI)->getT();
        filename.clear();
        ros << "scc-" << F.getName() << "-" << (count++) << ".dot";
        writeGraph<SCC>(ros.str(), scc);
      }

      delete sccSubgraph;
      delete subgraph;

      /*
       * Name and graph the loop DG of the function
       */
      LoopInfo& LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
      if (LI.empty()) return ;
      filename.clear();
      ros << "pdg-" << F.getName() << "-loop1.dot";

      subgraph = graph->createLoopsSubgraph(*(LI.getLoopsInPreorder().begin()));
      writeGraph<PDG>(ros.str(), subgraph);
      delete subgraph;
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
