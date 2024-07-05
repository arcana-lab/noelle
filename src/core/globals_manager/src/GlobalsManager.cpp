/*
 * Copyright 2022 - 2023  Simone Campanoni
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
#include "arcana/noelle/core/GlobalsManager.hpp"

namespace arcana::noelle {

GlobalsManager::GlobalsManager(Module &m) : program{ m } {
  return;
}

std::set<GlobalVariable *> GlobalsManager::getGlobals(void) const {
  std::set<GlobalVariable *> gs;

  /*
   * Find all globals.
   */
  auto &M = this->program;
  for (llvm::Module::global_iterator GI = M.global_begin(), GE = M.global_end(); GI != GE; ++GI) {
    auto &g = *GI;
    gs.insert(&g);
  }

  return gs;
}

} // namespace arcana::noelle
