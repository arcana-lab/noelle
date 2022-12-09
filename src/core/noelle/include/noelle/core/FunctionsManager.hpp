/*
 * Copyright 2020 - 2022  Simone Campanoni
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
#include "noelle/core/PDGAnalysis.hpp"
#include "noelle/core/Hot.hpp"

namespace llvm::noelle {

class FunctionsManager {
public:
  FunctionsManager(Module &m, PDGAnalysis &noellePDGAnalysis, Hot *profiles);

  Function *getEntryFunction(void) const;

  std::set<Function *> getProgramConstructors(void) const;

  bool isTheLibraryFunctionPure(Function *libraryFunction);

  bool canModifyMemory(Function &f);

  Function *getFunction(const std::string &name);

  CallGraph *getProgramCallGraph(void);

  Function *newFunction(const std::string &name, FunctionType &signature);

  std::set<Function *> getFunctions(void) const;

  std::set<Function *> getFunctionsReachableFrom(Function *startingPoint);

  void sortByHotness(std::vector<Function *> &functions);

  void removeFunction(Function &f);

private:
  std::set<Function *> nonMemModifiers;
  bool nonMemModifiersIsInitialized;
  Module &program;
  PDGAnalysis &pdgAnalysis;
  CallGraph *pcg;
  Hot *prof;
};

} // namespace llvm::noelle
