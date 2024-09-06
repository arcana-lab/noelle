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
#include "arcana/noelle/core/MemoryCloningAnalysis.hpp"

namespace arcana::noelle {

MemoryCloningAnalysis::MemoryCloningAnalysis(LoopStructure *loop,
                                             DominatorSummary &DS,
                                             PDG *ldg)
  : log(NoelleLumberjack, "MemoryCloningAnalysis") {
  assert(loop != nullptr);
  assert(ldg != nullptr);
  log.info() << "Start\n";

  /*
   * Collect objects allocated on the stack.
   * We assume such objects are allocated at the entry point of the function.
   */
  std::unordered_set<AllocaInst *> allocations;
  auto function = loop->getFunction();
  auto &entryBlock = function->getEntryBlock();
  for (auto &I : entryBlock) {
    auto alloca = dyn_cast<AllocaInst>(&I);
    if (alloca == nullptr) {
      continue;
    }
    allocations.insert(alloca);
  }

  /*
   * Now we need to check whether we can determine at compile time the size of
   * the stack objects.
   *
   * To do this, we first need to fetch the data layout.
   */
  auto &DL = function->getParent()->getDataLayout();

  /*
   * Check each stack object's size.
   */
  for (auto allocation : allocations) {

    /*
     * Check if we know the size in bits of the stack object.
     */
    auto sizeInBitsOptional = allocation->getAllocationSizeInBits(DL);
    if (!sizeInBitsOptional.hasValue()) {
      continue;
    }

    /*
     * Fetch the size of the stack object.
     */
    auto sizeInBits = sizeInBitsOptional.getValue();

    /*
     * Check if the stack object is clonable.
     */
    auto location = std::make_unique<ClonableMemoryObject>(allocation,
                                                           sizeInBits,
                                                           loop,
                                                           DS,
                                                           ldg);
    if (!location->isClonableLocation()) {
      continue;
    }

    /*
     * The stack object is clonable.
     */
    errs() << "MemoryCloningAnalysis:   The stack object "
           << *location->getAllocation() << " can be cloned\n";
    if (location->doPrivateCopiesNeedToBeInitialized()) {
      errs()
          << "MemoryCloningAnalysis:     The private copies need to be initialized with the original object.\n";
    }
    this->clonableMemoryLocations.insert(std::move(location));
  }

  errs() << "MemoryCloningAnalysis: Exit\n";
  return;
}

std::unordered_set<ClonableMemoryObject *> MemoryCloningAnalysis::
    getClonableMemoryObjects(void) const {
  std::unordered_set<ClonableMemoryObject *> locations{};
  for (auto &location : this->clonableMemoryLocations) {
    locations.insert(location.get());
  }
  return locations;
}

const std::unordered_set<ClonableMemoryObject *> MemoryCloningAnalysis::
    getClonableMemoryObjectsFor(Instruction *I) const {
  std::unordered_set<ClonableMemoryObject *> locs = {};

  /*
   * TODO: Determine if it is worth mapping from instructions to locations
   */
  for (auto &location : this->clonableMemoryLocations) {
    if (location->getAllocation() == I) {
      locs.insert(location.get());
    }
    if (location->isInstructionCastOrGEPOfLocation(I)) {
      locs.insert(location.get());
    }
    if (location->isInstructionLoadingLocation(I)) {
      locs.insert(location.get());
    }
    if (location->isInstructionStoringLocation(I)) {
      locs.insert(location.get());
    }
    if (auto callInst = dyn_cast<CallInst>(I)) {
      if (callInst->isLifetimeStartOrEnd()) {
        auto ptr = callInst->getArgOperand(1);
        auto loc = location.get();
        if (loc->mustAliasAMemoryLocationWithinObject(ptr)) {
          locs.insert(loc);
        }
      }
    }
  }

  return locs;
}

} // namespace arcana::noelle
