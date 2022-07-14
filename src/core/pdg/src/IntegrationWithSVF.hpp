/*
 * Copyright 2020 - 2021  Simone Campanoni
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
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/CallGraph.hpp"

namespace llvm::noelle {

class NoelleSVFIntegration : public ModulePass {
public:
  static char ID;

  NoelleSVFIntegration();
  bool doInitialization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnModule(Module &M) override;

  static noelle::CallGraph *getProgramCallGraph(Module &M);
  static bool hasIndCSCallees(CallBase *call);
  static const std::set<const Function *> getIndCSCallees(CallBase *call);
  static bool isReachableBetweenFunctions(const Function *from,
                                          const Function *to);
  static ModRefInfo getModRefInfo(CallBase *i);
  static ModRefInfo getModRefInfo(CallBase *i, const MemoryLocation &loc);
  static ModRefInfo getModRefInfo(CallBase *i, CallBase *j);
  static AliasResult alias(const MemoryLocation &loc1,
                           const MemoryLocation &loc2);
  static AliasResult alias(const Value *v1, const Value *v2);
};

} // namespace llvm::noelle
