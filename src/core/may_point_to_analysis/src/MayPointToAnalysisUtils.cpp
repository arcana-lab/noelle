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
#include "noelle/core/MayPointToAnalysis.hpp"
#include "MayPointToAnalysisUtils.hpp"

using namespace std;

const set<std::string> READ_ONLY_LIB_FUNCTIONS = {
  "atoi",   "atof",    "atol",   "atoll",  "fprintf", "fputc", "fputs",
  "putc",   "putchar", "printf", "puts",   "rand",    "scanf", "sqrt",
  "strlen", "strncmp", "strtod", "strtol", "strtoll"
};

const set<std::string> READ_ONLY_LIB_FUNCTIONS_WITH_SUFFIX =
    []() -> std::set<std::string> {
  std::set<std::string> result;
  for (auto fname : READ_ONLY_LIB_FUNCTIONS) {
    result.insert(fname);
    result.insert(fname + "_unlocked");
  }
  return result;
}();

namespace llvm::noelle {

Value *strip(Value *pointer) {
  return pointer->stripPointerCasts();
}

MemoryObjects unite(const MemoryObjects &lhs, const MemoryObjects &rhs) {
  MemoryObjects result(lhs);
  result.insert(rhs.begin(), rhs.end());
  return result;
}

MemoryObjects minus(const MemoryObjects &lhs, const MemoryObjects &rhs) {
  MemoryObjects result(lhs);
  for (auto v : rhs)
    result.erase(v);
  return result;
}

MemoryObjects intersect(const MemoryObjects &lhs, const MemoryObjects &rhs) {
  return minus(lhs, minus(lhs, rhs));
}

string getCalledFuncName(CallBase *callInst) {
  auto calledFunc = callInst->getCalledFunction();
  if (!calledFunc)
    return "";
  return calledFunc->getName();
}

bool isLifetimeIntrinsic(CallBase *callInst) {
  auto intrinsic = dyn_cast<IntrinsicInst>(callInst);
  if (!intrinsic) {
    return false;
  }
  return intrinsic->isLifetimeStartOrEnd();
}

MPAFunctionType getMPAFunctionType(CallBase *callInst) {
  auto calledFunc = callInst->getCalledFunction();
  auto fname = getCalledFuncName(callInst);

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
  } else if ((calledFunc != nullptr) && (!calledFunc->isDeclaration())) {
    return USER_DEFINED;
  } else {
    return UNKNOWN;
  }
};

} // namespace llvm::noelle