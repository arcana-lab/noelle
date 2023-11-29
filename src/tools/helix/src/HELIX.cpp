/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
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
#include "noelle/core/Architecture.hpp"
#include "noelle/core/ReductionSCC.hpp"
#include "noelle/tools/HELIX.hpp"
#include "noelle/tools/DOALL.hpp"

namespace arcana::noelle {

HELIX::HELIX(Noelle &n, bool forceParallelization)
  : ParallelizationTechniqueForLoopsWithLoopCarriedDataDependences{ n,
                                                                    forceParallelization },
    loopCarriedLoopEnvironmentBuilder{ nullptr },
    lastIterationExecutionBlock{ nullptr },
    enableInliner{ true },
    prefixString{ "HELIX: " } {

  /*
   * Fetch the LLVM context.
   */
  auto program = this->noelle.getProgram();

  /*
   * Fetch the dispatcher to use to jump to a parallelized HELIX loop.
   */
  this->taskDispatcherSS =
      program->getFunction("NOELLE_HELIX_dispatcher_sequentialSegments");
  assert(this->taskDispatcherSS != nullptr);
  this->taskDispatcherCS =
      program->getFunction("NOELLE_HELIX_dispatcher_criticalSections");
  assert(this->taskDispatcherCS != nullptr);
  this->waitSSCall = program->getFunction("HELIX_wait");
  this->signalSSCall = program->getFunction("HELIX_signal");

  return;
}

Function *HELIX::getTaskFunction(void) const {
  assert(tasks.size() > 0);
  return tasks[0]->getTaskBody();
}

uint32_t HELIX::getMinimumNumberOfIdleCores(void) const {
  return Architecture::getNumberOfPhysicalCores();
  if (this->originalLDI == nullptr) {
    abort();
  }
  auto ltm = this->originalLDI->getLoopTransformationsManager();

  return ltm->getMaximumNumberOfCores();
}

std::string HELIX::getName(void) const {
  return "HELIX";
}

Transformation HELIX::getParallelizationID(void) const {
  return Transformation::HELIX_ID;
}

HELIX::~HELIX() {
  return;
}

} // namespace arcana::noelle
