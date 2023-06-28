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
#pragma once

#include "noelle/core/MayPointToAnalysis.hpp"

namespace llvm::noelle {

Value *strip(Value *pointer);

const std::set<std::string> READ_ONLY_LIB_FUNCTIONS = {
  "atoi",   "atof",    "atol",   "atoll",  "fprintf", "fputc", "fputs",
  "putc",   "putchar", "printf", "puts",   "rand",    "scanf", "sqrt",
  "strlen", "strncmp", "strtod", "strtol", "strtoll"
};

const std::set<std::string> READ_ONLY_LIB_FUNCTIONS_WITH_SUFFIX =
    []() -> std::set<std::string> {
  std::set<std::string> result;
  for (auto fname : READ_ONLY_LIB_FUNCTIONS) {
    result.insert(fname);
    result.insert(fname + "_unlocked");
  }
  return result;
}();

enum MPAFunctionType {
  MALLOC,
  CALLOC,
  REALLOC,
  FREE,
  INTRINSIC,
  READ_ONLY,
  MEM_COPY,
  USER_DEFINED,
  UNKNOWN
};

MemoryObjects intersect(const MemoryObjects &lhs, const MemoryObjects &rhs);

MemoryObjects unite(const MemoryObjects &lhs, const MemoryObjects &rhs);

MemoryObjects minus(const MemoryObjects &lhs, const MemoryObjects &rhs);

MemoryObjects replace(const MemoryObjects &memObjSet,
                      MemoryObject *oldObj,
                      MemoryObject *newObj);

// PointToGraph add(const PointToGraph &lhs, const PointToGraph &rhs);

// PointToGraph minus(const PointToGraph &lhs, const PointToGraph &rhs);

bool isLifetimeIntrinsic(CallBase *callInst);

MPAFunctionType getMPAFunctionType(CallBase *callInst);

} // namespace llvm::noelle