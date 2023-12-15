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
#ifndef NOELLE_SRC_CORE_CALL_GRAPH_ANALYSIS_CALL_GRAPH_ANALYSIS_H
#define NOELLE_SRC_CORE_CALL_GRAPH_ANALYSIS_CALL_GRAPH_ANALYSIS_H

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/DGBase.hpp"
#include "noelle/core/LoopStructure.hpp"

namespace arcana::noelle {

enum CallStrength { CS_CANNOT_EXIST, CS_MAY_EXIST, CS_MUST_EXIST };

class CallGraphAnalysis {
public:
  CallGraphAnalysis(const std::string &name);

  std::string getName(void) const;

  virtual CallStrength canThisFunctionBeACallee(Function &caller,
                                                Function &potentialCallee);

  virtual CallStrength canThisFunctionBeACallee(CallBase *caller,
                                                Function &potentialCallee);

  virtual ~CallGraphAnalysis();

private:
  std::string analysisName;
};

} // namespace arcana::noelle

#endif
