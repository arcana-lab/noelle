/*
 * Copyright 2024 - Federico Sossai, Simone Campanoni
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
#include <string>

#include "arcana/noelle/core/NoellePass.hpp"
#include "arcana/noelle/core/MultiExitRegionTree.hpp"

using namespace std;
using namespace llvm;
using namespace arcana::noelle;

namespace arcana::noelle {

class Pragma : public ModulePass {
public:
  static char ID;
  Pragma();
  ~Pragma() = default;
  bool doInitialization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnModule(Module &M) override;

private:
  string prefix;
};

Pragma::Pragma() : ModulePass{ ID }, prefix("Pragma: ") {}

bool Pragma::doInitialization(Module &M) {
  return false;
}

void Pragma::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<NoellePass>();
  return;
}

bool Pragma::runOnModule(Module &M) {
  errs() << prefix << "Start\n";

  // auto &noelle = getAnalysis<NoellePass>().getNoelle();
  auto &MainF = *M.getFunction("main");

  MultiExitRegionTree MERT(MainF, {}, {});

  errs() << prefix << "End\n";
  return false;
}

} // namespace arcana::noelle

char Pragma::ID = 0;
static RegisterPass<Pragma> X("Pragma", "");
