/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
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
#include "noelle/tools/DOALL.hpp"

namespace arcana::noelle {

DOALL::DOALL(Noelle &noelle)
  : ParallelizationTechnique{ noelle },
    enabled{ true },
    taskDispatcher{ nullptr },
    n{ noelle } {

  /*
   * Fetch the dispatcher to use to jump to a parallelized DOALL loop.
   */
  this->taskDispatcher =
      this->n.getProgram()->getFunction("NOELLE_DOALLDispatcher");
  if (this->taskDispatcher == nullptr) {
    this->enabled = false;
    if (this->verbose != Verbosity::Disabled) {
      errs()
          << "DOALL: WARNING: function NOELLE_DOALLDispatcher couldn't be found. DOALL is disabled\n";
    }
  }

  return;
}

uint32_t DOALL::getMinimumNumberOfIdleCores(void) const {
  return 2;
}

std::string DOALL::getName(void) const {
  return "DOALL";
}

Transformation DOALL::getParallelizationID(void) const {
  return Transformation::DOALL_ID;
}

} // namespace arcana::noelle
