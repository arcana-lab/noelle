/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"
#include "CleanMetadata.hpp"

static llvm::cl::opt<bool> CleanPDGMetadata("clean-pdg-metadata", llvm::cl::init(false), llvm::cl::desc("Clean metadata of pdg"));

using namespace llvm;

bool llvm::CleanMetadata::doInitialization(Module &M) {
  return false;
}

void llvm::CleanMetadata::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  return;
}

bool llvm::CleanMetadata::runOnModule(Module &M) {
  if (CleanPDGMetadata) {
    cleanPDGMetadata(M);
  }

  return false;
}

llvm::CleanMetadata::CleanMetadata()
  : ModulePass{ID} {
  return;
}

llvm::CleanMetadata::~CleanMetadata() {
  return;
}

void llvm::CleanMetadata::cleanPDGMetadata(Module &M) {
  errs() << "Clean PDG Metadata\n";

  for (auto &F : M) {
    if (F.hasMetadata("pdg.args.id")) {
      F.setMetadata("pdg.args.id", nullptr);
    }
    if (F.hasMetadata("pdg.edges")) {
      F.setMetadata("pdg.edges", nullptr);
    }
    
    for (auto &B : F) {
      for (auto &I : B) {
        if (I.getMetadata("pdg.inst.id")) {
          I.setMetadata("pdg.inst.id", nullptr);
        }
      }
    }
  }

  if (NamedMDNode *n = M.getNamedMetadata("module.pdg")) {
    M.eraseNamedMetadata(n);
  }

  return;
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
