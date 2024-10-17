/*
 * Copyright 2016 - 2024 Simone Campanoni
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
#ifndef NOELLE_SRC_TOOLS_SCCPRINTER_H
#define NOELLE_SRC_TOOLS_SCCPRINTER_H

#include "arcana/noelle/core/NoellePass.hpp"
#include "arcana/noelle/core/Lumberjack.hpp"

namespace arcana::noelle {

std::string getSCCTypeName(GenericSCC::SCCKind type);

class SCCPrinter : public ModulePass {
public:
  static char ID;

  SCCPrinter();

  ~SCCPrinter() = default;

  bool doInitialization(Module &M) override;

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  void printLoopIDs(std::vector<LoopStructure *> *LSs);

  void printSCC(GenericSCC *scc);

private:
  std::vector<int> sccTypeWhiteList;
  std::vector<int> sccTypeBlackList;
  std::string targetFunctionName;
  bool printSCCInstructions;
  uint64_t targetLoopID;
  bool loopIDs;
  bool printDetails;
  Logger log;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_SCCPRINTER_H
