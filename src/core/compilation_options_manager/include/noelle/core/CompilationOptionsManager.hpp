/*
 * Copyright 2020 - 2023  Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_COMPILATION_OPTIONS_MANAGER_COMPILATIONOPTIONSMANAGER_H_
#define NOELLE_SRC_CORE_COMPILATION_OPTIONS_MANAGER_COMPILATIONOPTIONSMANAGER_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"

namespace arcana::noelle {

class CompilationOptionsManager {
public:
  CompilationOptionsManager(Module &m,
                            uint32_t maxCores,
                            bool arePRVGsNonDeterministic,
                            bool areFloatRealNumbers,
                            bool hoistLoopsToMain);

  uint32_t getMaximumNumberOfCores(void) const;

  bool canFloatsBeConsideredRealNumbers(void) const;

  bool arePRVGsNonDeterministic(void) const;

  bool shouldLoopsBeHoistToMain(void) const;

private:
  Module &program;
  uint32_t _maxCores;
  bool _arePRVGsNonDeterministic;
  bool _areFloatRealNumbers;
  bool _hoistLoopsToMain;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_COMPILATION_OPTIONS_MANAGER_COMPILATIONOPTIONSMANAGER_H_
