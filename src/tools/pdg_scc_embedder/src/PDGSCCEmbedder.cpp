/*
 * Copyright 2023 - 2024  Simone Campanoni
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
#include "PDGSCCEmbedder.hpp"
#include "arcana/noelle/core/NoellePass.hpp"

namespace arcana::noelle {

PDGSCCEmbedder::PDGSCCEmbedder() : ModulePass(ID) {
  return;
}

bool PDGSCCEmbedder::doInitialization(Module &M) {
  return false;
}

void PDGSCCEmbedder::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<NoellePass>();
  return;
}

bool PDGSCCEmbedder::runOnModule(Module &M) {

  /*
   * Fetch the NOELLE framework.
   */
  auto &noelle = getAnalysis<NoellePass>().getNoelle();

  /*
   * Get the PDG.
   */
  auto pdg = noelle.getProgramDependenceGraph();

  /*
   * Embed the PDG.
   */
  auto &pdgGen = noelle.getPDGGenerator();
  pdgGen.embedSCCAsMetadata(pdg);

  return true;
}

// Next there is code to register your pass to "opt"
char PDGSCCEmbedder::ID = 0;
static RegisterPass<PDGSCCEmbedder> X("PDGSCCEmbedder",
                                      "Embed the SCCs into the IR");

// Next there is code to register your pass to "clang"
static PDGSCCEmbedder *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
                                        [](const PassManagerBuilder &,
                                           legacy::PassManagerBase &PM) {
                                          if (!_PassMaker) {
                                            PM.add(_PassMaker =
                                                       new PDGSCCEmbedder());
                                          }
                                        }); // ** for -Ox
static RegisterStandardPasses _RegPass2(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new PDGSCCEmbedder());
      }
    }); // ** for -O0

} // namespace arcana::noelle
