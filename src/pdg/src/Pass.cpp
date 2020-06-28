/*
 * Copyright 2016 - 2020  Angelo Matni, Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"

#include "PDGAnalysis.hpp"
#include "PDGPrinter.hpp"

using namespace llvm;

/*
 * Pass options.
 */
static cl::opt<int> PDGVerbose("noelle-pdg-verbose", cl::ZeroOrMore, cl::Hidden, cl::desc("Verbose output (0: disabled, 1: minimal, 2: maximal, 3:maximal plus dumping PDG"));
static cl::opt<bool> PDGEmbed("noelle-pdg-embed", cl::ZeroOrMore, cl::Hidden, cl::desc("Embed the PDG"));
static cl::opt<bool> PDGDump("noelle-pdg-dump", cl::ZeroOrMore, cl::Hidden, cl::desc("Dump the PDG"));

bool PDGAnalysis::doInitialization (Module &M){
  this->verbose = static_cast<PDGVerbosity>(PDGVerbose.getValue());
  this->embedPDG = (PDGEmbed.getNumOccurrences() > 0) ? true : false;
  this->dumpPDG = (PDGDump.getNumOccurrences() > 0) ? true : false;

  return false;
}

void PDGAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<CallGraphWrapperPass>();
  AU.addRequired<AllocAA>();
  AU.addRequired<TalkDown>();
  AU.setPreservesAll();

  return ;
}

bool PDGAnalysis::runOnModule (Module &M){

  /*
   * Store global information.
   */
  this->M = &M;

  /*
   * Initialize DataFlowAnalysis.
   */
  this->dfa = new DataFlowAnalysis();

  /*
   * Initialize SVF.
   */
  initializeSVF(M);

  /*
   * Function reachability analysis.
   */
  identifyFunctionsThatInvokeUnhandledLibrary(M);

  /*
   * Construct PDG.
   */
  auto currentPDG = this->getPDG();

  /*
   * Check if we should dumpt the PDG
   */
  if (this->dumpPDG){

    /*
     * Dump the PDG
     */
    auto localPDGPrinter = new PDGPrinter();
    auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
    auto getLoopInfo = [this](Function *f) -> LoopInfo& {
      auto& LI = getAnalysis<LoopInfoWrapperPass>(*f).getLoopInfo();
      return LI;
    };
    localPDGPrinter->printPDG(M, callGraph, currentPDG, getLoopInfo);
  }

  return false;
}
