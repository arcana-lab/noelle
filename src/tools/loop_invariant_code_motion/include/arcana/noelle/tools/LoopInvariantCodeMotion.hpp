/*
 * Copyright 2019 - 2021  Simone Campanoni
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
#ifndef NOELLE_SRC_TOOLS_LOOP_INVARIANT_CODE_MOTION_LOOPINVARIANTCODEMOTION_H_
#define NOELLE_SRC_TOOLS_LOOP_INVARIANT_CODE_MOTION_LOOPINVARIANTCODEMOTION_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/Noelle.hpp"
#include "arcana/noelle/core/PDGPrinter.hpp"
#include "arcana/noelle/core/SubCFGs.hpp"

namespace arcana::noelle {

class LoopInvariantCodeMotion {
public:
  /*
   * Methods
   */
  LoopInvariantCodeMotion(Noelle &noelle);

  bool extractInvariantsFromLoop(LoopContent const &LC);

  bool promoteMemoryLocationsToRegisters(LoopContent const &LC);

private:
  /*
   * Fields
   */
  Noelle &noelle;

  /*
   * Methods
   */
  bool hoistStoreOfLastValueLiveOut(LoopContent const &LC);

  bool hoistInvariantValues(LoopContent const &LC);

  std::vector<Instruction *> getSourceDependenceInstructionsFrom(
      LoopContent const &LC,
      Instruction &I);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_LOOP_INVARIANT_CODE_MOTION_LOOPINVARIANTCODEMOTION_H_
