/*
 * Copyright 2016 - 2024  Yian Su, Simone Campanoni
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
#include "MetadataCleaner.hpp"
#include "arcana/noelle/core/NoellePass.hpp"

namespace arcana::noelle {

static cl::opt<bool> CleanLoopMetadata(
    "clean-loop-metadata",
    cl::init(false),
    cl::desc("noelle/core/Clean metadata of loops"));
static cl::opt<bool> CleanPDGMetadata(
    "clean-pdg-metadata",
    cl::init(false),
    cl::desc("noelle/core/Clean metadata of pdg"));
static cl::opt<bool> CleanSCCMetadata(
    "clean-pdg-scc-metadata",
    cl::init(false),
    cl::desc("noelle/core/Clean metadata of pdg scc"));
static cl::opt<bool> CleanProfileMetadata(
    "clean-prof-metadata",
    cl::init(false),
    cl::desc("noelle/core/Clean metadata of profiles"));

MetadataCleaner::MetadataCleaner() : cleanPDG{ false } {

  this->cleanLoop = CleanLoopMetadata.getNumOccurrences() > 0 ? true : false;
  this->cleanPDG = CleanPDGMetadata.getNumOccurrences() > 0 ? true : false;
  this->cleanSCC = CleanSCCMetadata.getNumOccurrences() > 0 ? true : false;
  this->cleanProf = CleanProfileMetadata.getNumOccurrences() > 0 ? true : false;

  return;
}

PreservedAnalyses MetadataCleaner::run(Module &M,
                                       llvm::ModuleAnalysisManager &AM) {
  if (this->cleanLoop) {
    this->cleanLoopMetadata(M);
  }

  if (this->cleanPDG) {
    this->cleanPDGMetadata(M);
  }

  if (this->cleanSCC) {
    this->cleanSCCMetadata(M);
  }

  if (this->cleanProf) {
    this->cleanProfMetadata(M);
  }

  return PreservedAnalyses::none();
}

// Next there is code to register your pass to "opt"
llvm::PassPluginLibraryInfo getPluginInfo() {
  return { LLVM_PLUGIN_API_VERSION,
           "MetadataCleaner",
           LLVM_VERSION_STRING,
           [](PassBuilder &PB) {
             /*
              * REGISTRATION FOR "opt -passes='MetadataCleaner'"
              *
              */
             PB.registerPipelineParsingCallback(
                 [](StringRef Name,
                    llvm::ModulePassManager &PM,
                    ArrayRef<llvm::PassBuilder::PipelineElement>) {
                   if (Name == "MetadataCleaner") {
                     PM.addPass(MetadataCleaner());
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
