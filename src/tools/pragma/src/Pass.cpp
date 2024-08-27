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

#include "arcana/noelle/core/MultiExitRegionTree.hpp"
#include "arcana/noelle/core/PragmaManager.hpp"

#include "Pass.hpp"

using namespace std;
using namespace llvm;
using namespace arcana::noelle;

static cl::opt<string> Directive(
    "noelle-pragma-directive",
    cl::init(""),
    cl::value_desc("string"),
    cl::desc("Name of the directive to use to search to SEME regions"));

static cl::opt<string> FunctionName("noelle-pragma-function",
                                    cl::init(""),
                                    cl::desc("Scan only a given function"));

namespace arcana::noelle {

Pragma::Pragma() : ModulePass{ ID }, prefix("Pragma: ") {}

bool Pragma::doInitialization(Module &M) {
  return false;
}

void Pragma::getAnalysisUsage(AnalysisUsage &AU) const {
  return;
}

bool Pragma::runOnModule(Module &M) {
  bool scanAllFunctions = FunctionName == "";

  for (auto &F : M) {
    if (F.empty()) {
      continue;
    }

    if (scanAllFunctions || F.getName() == FunctionName) {
      PragmaManager PM(F, Directive);
      if (!PM.getPragmaTree()->isEmpty()) {
        PM.print(errs(), prefix);
        errs() << prefix << "\n";
        errs() << prefix << "Iterator:\n";
        for (auto T : PM.getPragmaTree()->preOrderTraversal()) {
          errs() << prefix << PM.getRegionDirective(T) << "\n";
        }
      }
    }
  }

  return false;
}

} // namespace arcana::noelle

char Pragma::ID = 0;
static RegisterPass<Pragma> X(
    "Pragma",
    "Print region trees for a given pragma directive");
