/*
 * Copyright 2020 - 2021  Simone Campanoni
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
#include "noelle/core/CompilationOptionsManager.hpp"

namespace llvm::noelle {

CompilationOptionsManager::CompilationOptionsManager(
    Module &m,
    uint32_t maxCores,
    bool arePRVGsNonDeterministic,
    bool areFloatRealNumbers,
    bool hoistLoopsToMain)
  : program{ m },
    _maxCores{ maxCores },
    _arePRVGsNonDeterministic{ arePRVGsNonDeterministic },
    _areFloatRealNumbers{ areFloatRealNumbers },
    _hoistLoopsToMain{ hoistLoopsToMain } {
  return;
}

uint32_t CompilationOptionsManager::getMaximumNumberOfCores(void) const {
  return this->_maxCores;
}

bool CompilationOptionsManager::canFloatsBeConsideredRealNumbers(void) const {
  return this->_areFloatRealNumbers;
}

bool CompilationOptionsManager::arePRVGsNonDeterministic(void) const {
  return this->_arePRVGsNonDeterministic;
}

bool CompilationOptionsManager::shouldLoopsBeHoistToMain(void) const {
  return this->_hoistLoopsToMain;
}

} // namespace llvm::noelle
