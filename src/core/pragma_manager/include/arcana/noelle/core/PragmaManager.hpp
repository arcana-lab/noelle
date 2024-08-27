/*
 * Copyright 2021 - 2024 Federico Sossai, Simone Campanoni
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
#ifndef __PRAGMA_MANAGER_HPP__
#define __PRAGMA_MANAGER_HPP__

#include <string>

#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"
#include "arcana/noelle/core/MultiExitRegionTree.hpp"

namespace arcana::noelle {

class PragmaManager {
public:
  PragmaManager(llvm::Function &F, std::string directive = "");

  ~PragmaManager();

  static bool getStringFromArg(llvm::Value *arg, llvm::StringRef &result);

  MultiExitRegionTree *getPragmaTree();

  std::string getRegionDirective(MultiExitRegionTree *T) const;

  std::vector<llvm::Value *> getRegionArguments(MultiExitRegionTree *T) const;

  llvm::raw_ostream &print(llvm::raw_ostream &stream,
                           std::string prefixToUse = "",
                           bool printArgs = false);

private:
  llvm::Function &F;
  std::string directive;
  MultiExitRegionTree *MERT;

  enum SiblingType { INNER, LAST };

  llvm::raw_ostream &print(MultiExitRegionTree *T,
                           llvm::raw_ostream &stream,
                           std::string prefixToUse,
                           bool printArgs,
                           SiblingType ST);
};

} // namespace arcana::noelle

#endif
