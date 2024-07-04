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
#include "PDGEmbedder.hpp"
#include "arcana/noelle/core/NoellePass.hpp"

namespace arcana::noelle {

PreservedAnalyses PDGEmbedder::run(Module &M, llvm::ModuleAnalysisManager &AM) {

  /*
   * Fetch the NOELLE framework.
   */
  auto &noelle = AM.getResult<NoellePass>(M);

  /*
   * Get the PDG.
   */
  auto pdg = noelle.getProgramDependenceGraph();

  /*
   * Embed the PDG.
   */
  auto &pdgGen = noelle.getPDGGenerator();
  pdgGen.cleanAndEmbedPDGAsMetadata(pdg);

  return PreservedAnalyses::none();
}

// Next there is code to register your pass to "opt"
llvm::PassPluginLibraryInfo getPluginInfo() {
  return { LLVM_PLUGIN_API_VERSION,
           "PDGEmbedder",
           LLVM_VERSION_STRING,
           [](PassBuilder &PB) {
             /*
              * REGISTRATION FOR "opt -passes='PDGEmbedder'"
              *
              */
             PB.registerPipelineParsingCallback(
                 [](StringRef Name,
                    llvm::ModulePassManager &PM,
                    ArrayRef<llvm::PassBuilder::PipelineElement>) {
                   if (Name == "PDGEmbedder") {
                     PM.addPass(PDGEmbedder());
                     return true;
                   }
                   return false;
                 });

             /*
              * REGISTRATION FOR "AM.getResult<NoellePass>()"
              */
             PB.registerAnalysisRegistrationCallback(
                 [](ModuleAnalysisManager &AM) {
                   AM.registerPass([&] { return NoellePass(); });
                 });
           } };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getPluginInfo();
}

} // namespace arcana::noelle
