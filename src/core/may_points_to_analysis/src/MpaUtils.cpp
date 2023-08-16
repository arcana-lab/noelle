/*
 * Copyright 2023 Xiao Chen
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
#include "MpaUtils.hpp"

using namespace std;

namespace llvm::noelle {

MPAFunctionType getCalleeFunctionType(CallBase *callInst) {
  const set<string> READ_ONLY_LIB_FUNCTIONS = {
    "atoi",   "atof",    "atol",   "atoll",  "fprintf", "fputc", "fputs",
    "putc",   "putchar", "printf", "puts",   "rand",    "scanf", "sqrt",
    "strlen", "strncmp", "strtod", "strtol", "strtoll"
  };

  const set<string> READ_ONLY_LIB_FUNCTIONS_WITH_SUFFIX = [&]() -> set<string> {
    set<string> result;
    for (auto fname : READ_ONLY_LIB_FUNCTIONS) {
      result.insert(fname);
      result.insert(fname + "_unlocked");
    }
    return result;
  }();

  auto isLifetimeIntrinsic = [](CallBase *callInst) {
    auto intrinsic = dyn_cast<IntrinsicInst>(callInst);
    if (!intrinsic) {
      return false;
    }
    return intrinsic->isLifetimeStartOrEnd();
  };

  auto calleeFunc = callInst->getCalledFunction();
  auto fname = calleeFunc ? calleeFunc->getName() : "";

  if (fname == "malloc") {
    return MALLOC;
  } else if (fname == "calloc") {
    return CALLOC;
  } else if (fname == "realloc") {
    return REALLOC;
  } else if (fname == "free") {
    return FREE;
  } else if (isLifetimeIntrinsic(callInst)) {
    return INTRINSIC;
  } else if (READ_ONLY_LIB_FUNCTIONS_WITH_SUFFIX.count(fname) > 0) {
    return READ_ONLY;
  } else if (isa<MemCpyInst>(callInst)) {
    return MEM_COPY;
  } else if ((calleeFunc != nullptr) && (!calleeFunc->isDeclaration())) {
    return USER_DEFINED;
  } else {
    return UNKNOWN;
  }
}

Value *strip(Value *pointer) {
  assert(pointer != nullptr || pointer->getType()->isPointerTy());

  if (isa<GetElementPtrInst>(pointer)) {
    auto gepInst = dyn_cast<GetElementPtrInst>(pointer);
    return strip(gepInst->getPointerOperand());
  } else if (isa<GEPOperator>(pointer)) {
    auto gepOp = dyn_cast<GEPOperator>(pointer);
    return strip(gepOp->getPointerOperand());
  } else if (isa<BitCastInst>(pointer) || isa<BitCastOperator>(pointer)) {
    return strip(pointer->stripPointerCasts());
  } else {
    return pointer;
  }
}

BitVector unite(const BitVector &lhs, const BitVector &rhs) {
  BitVector result(lhs);
  result |= rhs;
  return result;
}

} // namespace llvm::noelle
