/*
 * Copyright 2019 - 2020  Simone Campanoni
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
#ifndef NOELLE_SRC_TOOLS_LOOP_INVARIANT_CODE_MOTION_MEM2REGNONALLOCA_H_
#define NOELLE_SRC_TOOLS_LOOP_INVARIANT_CODE_MOTION_MEM2REGNONALLOCA_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/LoopContent.hpp"
#include "arcana/noelle/core/Invariants.hpp"
#include "arcana/noelle/core/Noelle.hpp"
#include "arcana/noelle/core/PDGPrinter.hpp"
#include "arcana/noelle/core/SubCFGs.hpp"
#include "arcana/noelle/core/SCCDAG.hpp"

namespace arcana::noelle {

class Mem2RegNonAlloca {
public:
  Mem2RegNonAlloca(LoopContent const &LDI, Noelle &noelle);

  bool promoteMemoryToRegister(void);

private:
  LoopContent const &LDI;
  Noelle &noelle;
  InvariantManager &invariants;

  std::map<Value *, SCC *> findSCCsWithSingleMemoryLocations(void);

  bool hoistMemoryInstructionsRelyingOnExistingRegisterValues(
      SCC *scc,
      Value *memoryLocation);
  bool promoteMemoryToRegisterForSCC(SCC *scc, Value *memoryLocation);

  std::unordered_map<BasicBlock *, std::vector<Instruction *>>
  collectOrderedMemoryInstsByBlock(SCC *scc);

  void removeRedundantPHIs(
      std::unordered_set<PHINode *> phis,
      std::unordered_map<BasicBlock *, Value *> lastRegisterValueByBlock);

  /*
   * Can ONLY be used before stores/loads are erased, as this invalidates the
   * LDI
   * TODO: Store a flag indicating whether LDI is invalidated
   */
  void assertAndDumpLogsOnFailure(std::function<bool(void)> assertion,
                                  std::string errorString);
  void dumpLogs(void);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_LOOP_INVARIANT_CODE_MOTION_MEM2REGNONALLOCA_H_
