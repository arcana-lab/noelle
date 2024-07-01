/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni
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
#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/Noelle.hpp"

namespace arcana::noelle {

DominatorSummary *Noelle::getDominators(Function *f) {

  /*
   * Fetch the dominators from LLVM.
   */
  auto &DT = this->getDT(*f);
  auto &PDT = this->getPDT(*f);

  /*
   * Combine them.
   */
  auto ds = new DominatorSummary(DT, PDT);

  return ds;
}

FunctionsManager *Noelle::getFunctionsManager(void) {
  if (!this->fm) {
    this->fm = new FunctionsManager(this->program,
                                    this->pdgAnalysis,
                                    this->getProfiles());
  }
  return this->fm;
}

void Noelle::addAnalysis(CallGraphAnalysis *a) {
  this->pdgAnalysis.addAnalysis(a);

  return;
}

} // namespace arcana::noelle
