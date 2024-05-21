/*
 * Copyright 2024 Sophia Boksenbaum, Simone Campanoni
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
#include "noelle/core/LoopCarriedSCC.hpp"
#include "noelle/core/LoopContent.hpp"
#include "noelle/core/Dominators.hpp"

namespace arcana::noelle {

class OutputSequenceSCC : public LoopCarriedSCC {
public:
  OutputSequenceSCC(
      SCC *s,
      LoopStructure *loop,
      const std::set<DGEdge<Value, Value> *> &loopCarriedDependences);

  static std::optional<uint32_t> getMaxNumberOfPrintedBytes(CallInst *);

  static bool classof(const GenericSCC *);

protected:
  static std::optional<std::string> printConstantFstring(CallInst *);
  static std::optional<uint32_t> fstringMaxLength(std::string);
};

} // namespace arcana::noelle
