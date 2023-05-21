/*
 * Copyright 2021 - 2022  Simone Campanoni
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
#include "noelle/core/ConstantsManager.hpp"

namespace llvm::noelle {

ConstantsManager::ConstantsManager(Module &m, TypesManager *tm)
  : program{ m },
    tm{ tm } {
  return;
}

Constant *ConstantsManager::getIntegerConstant(int64_t constantValue,
                                               uint32_t bitwidth) const {

  /*
   * Fetch the type
   */
  auto t = this->tm->getIntegerType(bitwidth);

  /*
   * Fetch the constant
   */
  auto c = this->getIntegerConstant(constantValue, t);

  return c;
}

Constant *ConstantsManager::getIntegerConstant(int64_t constantValue,
                                               Type *integerType) const {

  /*
   * Fetch the constant
   */
  auto c = ConstantInt::get(integerType, constantValue);

  return c;
}

} // namespace llvm::noelle
