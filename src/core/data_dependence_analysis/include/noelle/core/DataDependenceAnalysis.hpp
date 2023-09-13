/*
 * Copyright 2023 - 2024  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publoopsh, distribute, sublicense, and/or sell copies
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
#include "noelle/core/DGBase.hpp"

namespace llvm::noelle {

enum MemoryDataDependenceStrength { CANNOT_EXIST, MAY_EXIST, MUST_EXIST };

class DataDependenceAnalysis {
public:
  DataDependenceAnalysis(const std::string &name);

  std::string getName(void) const;

  virtual bool canThereBeAMemoryDataDependence(Instruction *fromInst,
                                               Instruction *toInst) = 0;

  virtual MemoryDataDependenceStrength isThereThisMemoryDataDependenceType(
      DataDependenceType t,
      Instruction *fromInst,
      Instruction *toInst);

private:
  std::string analysisName;
};

} // namespace llvm::noelle
