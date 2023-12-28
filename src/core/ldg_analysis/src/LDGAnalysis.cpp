/*
 * Copyright 2023 - 2024  Simone Campanoni
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
#include "noelle/core/LDGAnalysis.hpp"

namespace arcana::noelle {

LDGAnalysis::LDGAnalysis() {
  return;
}

void LDGAnalysis::addAnalysis(DependenceAnalysis *a) {
  this->ddAnalyses.insert(a);
}

void LDGAnalysis::improveDependenceGraph(PDG *loopDG, LoopStructure *loop) {

  this->removeDependences(loopDG, loop);
}

void LDGAnalysis::removeDependences(PDG *loopDG, LoopStructure *loop) {

  /*
   * Fetch all dependences.
   */
  auto deps = loopDG->getSortedDependences();

  /*
   * Identify dependences to remove.
   */
  std::set<DGEdge<Value, Value> *> toDelete{};
  for (auto dep : deps) {

    /*
     * Fetch the two instructions that depend on each other.
     */
    auto s = dep->getSrc();
    auto d = dep->getDst();
    auto srcInst = dyn_cast<Instruction>(s);
    auto dstInst = dyn_cast<Instruction>(d);
    if (srcInst == nullptr) {
      continue;
    }
    if (dstInst == nullptr) {
      continue;
    }

    /*
     * Make sure we only check dependences between instructions that are both
     * within the loop.
     */
    if (!loop->isIncluded(srcInst)) {
      continue;
    }
    if (!loop->isIncluded(dstInst)) {
      continue;
    }

    /*
     * We only aim to remove memory dependences.
     */
    if (!isa<MemoryDependence<Value, Value>>(dep)) {
      continue;
    }
    auto memDep = cast<MemoryDependence<Value, Value>>(dep);

    /*
     * We only aim to remove memory dependences that are "may"
     */
    if (memDep->isMustDependence()) {
      continue;
    }

    /*
     * Try to remove the current memory dependence.
     */
    for (auto dda : this->ddAnalyses) {
      if (!dda->canThereBeAMemoryDataDependence(srcInst, dstInst, *loop)) {
        toDelete.insert(memDep);
        break;
      }
      auto r = dda->isThereThisMemoryDataDependenceType(
          memDep->getDataDependenceType(),
          srcInst,
          dstInst,
          *loop);
      switch (r) {

        case MemoryDataDependenceStrength::CANNOT_EXIST:
          toDelete.insert(memDep);
          break;
      }
    }
  }

  /*
   * Remove dependences.
   */
  for (auto dep : toDelete) {
    loopDG->removeEdge(dep);
  }

  return;
}

void LDGAnalysis::removeLoopCarriedDependences(PDG *loopDG,
                                               LoopStructure *loop) {

  /*
   * Fetch all dependences.
   */
  auto deps = loopDG->getSortedDependences();

  /*
   * Identify dependences to remove.
   */
  for (auto dep : deps) {

    /*
     * Fetch the two instructions that depend on each other.
     */
    auto s = dep->getSrc();
    auto d = dep->getDst();
    auto srcInst = dyn_cast<Instruction>(s);
    auto dstInst = dyn_cast<Instruction>(d);
    if (srcInst == nullptr) {
      continue;
    }
    if (dstInst == nullptr) {
      continue;
    }

    /*
     * Make sure we only check dependences between instructions that are both
     * within the loop.
     */
    if (!loop->isIncluded(srcInst)) {
      continue;
    }
    if (!loop->isIncluded(dstInst)) {
      continue;
    }

    /*
     * We only target loop-carried dependences.
     */
    if (!dep->isLoopCarriedDependence()) {
      continue;
    }

    /*
     * Try to disprove the loop-carried property of the dependence.
     */
    for (auto dda : this->ddAnalyses) {
      if (!dda->canThisDependenceBeLoopCarried(dep, *loop)) {
        dep->setLoopCarried(false);
        break;
      }
    }
  }

  return;
}

} // namespace arcana::noelle
