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
#include "noelle/core/TypesManager.hpp"

namespace llvm::noelle {

TypesManager::TypesManager(Module &m) : program{ m } {
  return;
}

Type *TypesManager::getIntegerType(uint32_t bitwidth) const {
  auto &ctx = this->program.getContext();

  auto intType = IntegerType::get(ctx, bitwidth);

  return intType;
}

Type *TypesManager::getVoidPointerType(void) const {
  auto int8 = this->getIntegerType(8);
  auto t = PointerType::getUnqual(int8);

  return t;
}

Type *TypesManager::getVoidType(void) const {
  auto &ctx = this->program.getContext();

  auto voidType = Type::getVoidTy(ctx);

  return voidType;
}

uint64_t TypesManager::getSizeOfType(Type *t) const {
  assert(t != nullptr);

  if (!t->isSized()) {
    errs() << "NOELLE: ERROR = the type " << *t << " is not sized\n";
    abort();
  }
  uint64_t s = 0;
  if (t->isStructTy()) {
    abort();

  } else if (t->isArrayTy()) {
    auto numOfElements = t->getArrayNumElements();
    assert(numOfElements > 0);
    auto elemT = t->getArrayElementType();
    auto perElementSize = this->getSizeOfType(elemT);
    assert(perElementSize > 0);
    s = numOfElements * perElementSize;

  } else {
    s = t->getPrimitiveSizeInBits() / 8;
  }
  assert(s > 0);

  return s;
}

} // namespace llvm::noelle
