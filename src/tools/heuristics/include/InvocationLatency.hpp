/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_TOOLS_HEURISTICS_INVOCATIONLATENCY_H_
#define NOELLE_SRC_TOOLS_HEURISTICS_INVOCATIONLATENCY_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCDAGAttrs.hpp"
#include "noelle/core/SCCDAGPartition.hpp"
#include "noelle/core/Hot.hpp"

namespace arcana::noelle {

class InvocationLatency {
public:
  InvocationLatency(Hot *hot);

  uint64_t latencyPerInvocation(SCC *scc);

  uint64_t latencyPerInvocation(
      SCCDAGAttrs *,
      std::unordered_set<SCCSet *> &subsets,
      std::function<bool(GenericSCC *scc)> canBeRematerialized);

  uint64_t latencyPerInvocation(Instruction *inst);

  uint64_t queueLatency(Value *queueVal);

  std::set<Value *> &memoizeExternals(
      SCCDAGAttrs *,
      SCC *,
      std::function<bool(GenericSCC *scc)> canBeRematerialized);

  std::set<SCC *> &memoizeParents(
      SCCDAGAttrs *,
      SCC *,
      std::function<bool(GenericSCC *scc)> canBeRematerialized);

private:
  Hot *profiles;
  std::unordered_map<Function *, uint64_t> funcToCost;
  std::unordered_map<Value *, uint64_t> queueValToCost;
  std::unordered_map<SCC *, uint64_t> sccToCost;
  std::unordered_map<SCC *, std::set<Value *>> incomingExternals;
  std::unordered_map<SCC *, std::set<SCC *>> clonableParents;
};
} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_HEURISTICS_INVOCATIONLATENCY_H_
