/*
 * Copyright 2016 - 2021  Angelo Matni, Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/PDGGenerator.hpp"
#include "noelle/core/PDGPrinter.hpp"

namespace arcana::noelle {

/*
 * Pass options.
 */
static cl::opt<int> PDGVerbose(
    "noelle-pdg-verbose",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc(
        "Verbose output (0: disabled, 1: minimal, 2: maximal, 3:maximal plus dumping PDG"));

static cl::opt<bool> PDGEmbed("noelle-pdg-embed",
                              cl::ZeroOrMore,
                              cl::Hidden,
                              cl::desc("Embed the PDG"));

static cl::opt<bool> SCCEmbed("noelle-pdg-scc-embed",
                              cl::ZeroOrMore,
                              cl::Hidden,
                              cl::desc("Embed the SCCs"));

static cl::opt<bool> PDGDump("noelle-pdg-dump",
                             cl::ZeroOrMore,
                             cl::Hidden,
                             cl::desc("Dump the PDG"));

static cl::opt<bool> PDGCheck("noelle-pdg-check",
                              cl::ZeroOrMore,
                              cl::Hidden,
                              cl::desc("Check the PDG"));

static cl::opt<bool> PDGSVFDisable("noelle-disable-pdg-svf",
                                   cl::ZeroOrMore,
                                   cl::Hidden,
                                   cl::desc("Disable SVF"));

static cl::opt<bool> PDGSVFCallGraphDisable("noelle-disable-pdg-svf-callgraph",
                                            cl::ZeroOrMore,
                                            cl::Hidden,
                                            cl::desc("Disable SVF call graph"));

static cl::opt<bool> PDGAllocAADisable(
    "noelle-disable-pdg-allocaa",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Disable our custom alias analysis"));

static cl::opt<bool> PDGRADisable(
    "noelle-disable-pdg-reaching-analysis",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Disable the use of reaching analysis to compute the PDG"));

bool PDGGenerator::doInitialization(Module &M) {
  this->verbose = static_cast<PDGVerbosity>(PDGVerbose.getValue());
  this->embedPDG = (PDGEmbed.getNumOccurrences() > 0) ? true : false;
  this->embedSCC = (SCCEmbed.getNumOccurrences() > 0) ? true : false;
  this->dumpPDG = (PDGDump.getNumOccurrences() > 0) ? true : false;
  this->performThePDGComparison =
      (PDGCheck.getNumOccurrences() > 0) ? true : false;
  this->disableSVF = (PDGSVFDisable.getNumOccurrences() > 0) ? true : false;
  this->disableSVFCallGraph =
      (PDGSVFCallGraphDisable.getNumOccurrences() > 0) ? true : false;
  this->disableAllocAA =
      (PDGAllocAADisable.getNumOccurrences() > 0) ? true : false;
  this->disableRA = (PDGRADisable.getNumOccurrences() > 0) ? true : false;

  return false;
}

void PDGGenerator::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<AAResultsWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<AllocAA>();
  AU.addRequired<TalkDown>();
  AU.setPreservesAll();

  return;
}

bool PDGGenerator::runOnModule(Module &M) {

  /*
   * Check if the pass has already run.
   */
  if ((this->M != nullptr) || (this->programDependenceGraph != nullptr)) {
    return false;
  }

  /*
   * Store global information.
   */
  this->M = &M;

  /*
   * Initialize SVF.
   */
  initializeSVF(M);

  /*
   * Function reachability analysis.
   */
  identifyFunctionsThatInvokeUnhandledLibrary(M);

  /*
   * Check if we should compute the PDG.
   */
  if ((this->dumpPDG) || (this->embedPDG) || (this->embedSCC)) {

    /*
     * Construct PDG because this will trigger code that is needed by the
     * options specified.
     */
    this->getPDG();
  }

  return false;
}

} // namespace arcana::noelle
