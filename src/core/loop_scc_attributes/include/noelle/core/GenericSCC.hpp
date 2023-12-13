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
#ifndef NOELLE_SRC_CORE_LOOP_SCC_ATTRIBUTES_GENERICSCC_H_
#define NOELLE_SRC_CORE_LOOP_SCC_ATTRIBUTES_GENERICSCC_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/LoopStructure.hpp"

namespace arcana::noelle {

/*
 * This is the top of the class hierarchy that organizes SCCs of a loop.
 *
 * https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html
 */
class GenericSCC {
public:
  /*
   * Concrete sub-classes.
   */
  enum SCCKind {
    LOOP_CARRIED,

    REDUCTION,
    BINARY_REDUCTION,
    LAST_REDUCTION,

    RECOMPUTABLE,

    SINGLE_ACCUMULATOR_RECOMPUTABLE,

    INDUCTION_VARIABLE,
    LINEAR_INDUCTION_VARIABLE,
    LAST_INDUCTION_VARIABLE,

    PERIODIC_VARIABLE,

    LAST_SINGLE_ACCUMULATOR_RECOMPUTABLE,

    UNKNOWN_CLOSED_FORM,
    LAST_RECOMPUTABLE,

    MEMORY_CLONABLE,
    STACK_OBJECT_CLONABLE,
    LAST_MEMORY_CLONABLE,

    LOOP_CARRIED_UNKNOWN,

    LAST_LOOP_CARRIED,

    LOOP_ITERATION,

    LAST_LOOP_ITERATION
  };

  /*
   * Iterators.
   */
  typedef typename std::set<PHINode *>::iterator phi_iterator;
  typedef typename std::set<Instruction *>::iterator instruction_iterator;

  /*
   * Fields
   */
  std::set<Value *> stronglyConnectedDataValues;
  std::set<Value *> weaklyConnectedDataValues;

  /*
   * No public constructors.
   * Only objects of sub-classes can be allocated.
   */
  GenericSCC() = delete;

  /*
   * Get the SCC.
   */
  SCC *getSCC(void);

  /*
   * Get the PHIs.
   */
  iterator_range<phi_iterator> getPHIs(void) const;

  bool doesHaveMemoryDependencesWithin(void) const;

  SCCKind getKind(void) const;

  ~GenericSCC();

protected:
  LoopStructure *loop;
  SCC *scc;
  std::set<PHINode *> PHINodes;

  GenericSCC(SCCKind K, SCC *s, LoopStructure *loop);
  void collectPHIs(LoopStructure &LS);

private:
  SCCKind kind;
  bool hasMemoryDependences;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_SCC_ATTRIBUTES_GENERICSCC_H_
