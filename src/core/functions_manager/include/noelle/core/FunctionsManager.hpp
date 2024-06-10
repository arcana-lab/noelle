/*
 * Copyright 2020 - 2023  Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_FUNCTIONS_MANAGER_FUNCTIONSMANAGER_H_
#define NOELLE_SRC_CORE_FUNCTIONS_MANAGER_FUNCTIONSMANAGER_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/PDGGenerator.hpp"
#include "noelle/core/Hot.hpp"
#include "noelle/core/CallGraph.hpp"
#include "noelle/core/SCCCAG.hpp"

namespace arcana::noelle {

class FunctionsManager {
public:
  FunctionsManager(Module &m, PDGGenerator &noellePDGGenerator, Hot *profiles);

  Function *getEntryFunction(void) const;

  std::set<Function *> getProgramConstructors(void) const;

  bool isTheLibraryFunctionPure(Function *libraryFunction);

  Function *getFunction(const std::string &name);

  CallGraph *getProgramCallGraph(void);

  SCCCAG *getSCCDAGOfProgramCallGraph(void);

  Function *newFunction(const std::string &name, FunctionType &signature);

  std::set<Function *> getFunctions(void) const;

  std::set<Function *> getFunctionsWithPrefix(
      const std::string &prefixName) const;

  std::set<Function *> getFunctionsWithBody(void) const;

  std::set<Function *> getFunctionsReachableFrom(Function *startingPoint);

  void sortByHotness(std::vector<Function *> &functions);

  void removeFunction(Function &f);

private:
  Module &program;
  PDGGenerator &pdgAnalysis;
  CallGraph *pcg;
  Hot *prof;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_FUNCTIONS_MANAGER_FUNCTIONSMANAGER_H_
