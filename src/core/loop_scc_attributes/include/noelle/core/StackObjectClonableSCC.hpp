/*
 * Copyright 2022  Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_LOOP_SCC_ATTRIBUTES_STACKOBJECTCLONABLESCC_H_
#define NOELLE_SRC_CORE_LOOP_SCC_ATTRIBUTES_STACKOBJECTCLONABLESCC_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/MemoryClonableSCC.hpp"
#include "noelle/core/ClonableMemoryObject.hpp"

namespace arcana::noelle {

class StackObjectClonableSCC : public MemoryClonableSCC {
public:
  StackObjectClonableSCC(
      SCC *s,
      LoopStructure *loop,
      const std::set<DGEdge<Value, Value> *> &loopCarriedDependences,
      const std::set<ClonableMemoryObject *> &locations);

  StackObjectClonableSCC() = delete;

  /*
   * Return the memory locations that can be safely clone to void reusing the
   * same memory locations between invocations of this SCC.
   */
  std::set<AllocaInst *> getMemoryLocationsToClone(void) const;

  static bool classof(const GenericSCC *s);

protected:
  std::set<ClonableMemoryObject *> _clonableMemoryLocations;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_SCC_ATTRIBUTES_STACKOBJECTCLONABLESCC_H_
