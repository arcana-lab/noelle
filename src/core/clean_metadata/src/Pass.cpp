/*
 * Copyright 2016 - 2020  Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "CleanMetadata.hpp"

static cl::opt<bool> CleanPDGMetadata("clean-pdg-metadata", cl::init(false), cl::desc("Clean metadata of pdg"));
static cl::opt<bool> CleanProfileMetadata("clean-prof-metadata", cl::init(false), cl::desc("Clean metadata of profiles"));

using namespace llvm;

bool CleanMetadata::doInitialization(Module &M) {
  this->cleanPDG = CleanPDGMetadata.getNumOccurrences() > 0 ? true : false;
  this->cleanProf = CleanProfileMetadata.getNumOccurrences() > 0 ? true : false ;

  return false;
}

void CleanMetadata::getAnalysisUsage(AnalysisUsage &AU) const {
  return;
}

bool CleanMetadata::runOnModule(Module &M) {
  if (this->cleanPDG){
    this->cleanPDGMetadata(M);
  }

  if (this->cleanProf){
    this->cleanProfMetadata(M);
  }

  return false;
}

// Next there is code to register your pass to "opt"
char CleanMetadata::ID = 0;
static RegisterPass<CleanMetadata> X("CleanMetadata", "Clean the metadata embeded to the bitcode");

// Next there is code to register your pass to "clang"
static CleanMetadata * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CleanMetadata());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CleanMetadata()); }}); // ** for -O0
