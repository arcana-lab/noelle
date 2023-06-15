#pragma once

#include "noelle/core/MayPointToAnalysis.hpp"

#include <unordered_set>
#include <set>

namespace llvm::noelle {

Value *strip(Value *pointer);

const std::string MALLOC = "malloc";
const std::string CALLOC = "calloc";
const std::string REALLOC = "realloc";
const std::string FREE = "free";

const std::unordered_set<std::string> MEMORY_FUNCTIONS = { MALLOC,
                                                           CALLOC,
                                                           REALLOC,
                                                           FREE };

const std::unordered_set<std::string> READ_ONLY_LIB_FUNCTIONS = {
  "atoi",   "atof",    "atol",   "atoll",  "fprintf", "fputc", "fputs",
  "putc",   "putchar", "printf", "puts",   "rand",    "scanf", "sqrt",
  "strlen", "strncmp", "strtod", "strtol", "strtoll"
};

const std::unordered_set<std::string> READ_ONLY_LIB_FUNCTIONS_WITH_SUFFIX =
    []() -> std::unordered_set<std::string> {
  std::unordered_set<std::string> result;
  for (auto fname : READ_ONLY_LIB_FUNCTIONS) {
    result.insert(fname);
    result.insert(fname + "_unlocked");
  }
  return result;
}();

MemoryObjects intersect(const MemoryObjects &lhs, const MemoryObjects &rhs);

MemoryObjects unite(const MemoryObjects &lhs, const MemoryObjects &rhs);

MemoryObjects minus(const MemoryObjects &lhs, const MemoryObjects &rhs);

MemoryObjects replace(const MemoryObjects &memObjSet,
                      MemoryObject *oldObj,
                      MemoryObject *newObj);

PointToGraph add(const PointToGraph &lhs, const PointToGraph &rhs);

PointToGraph minus(const PointToGraph &lhs, const PointToGraph &rhs);

bool isLifetimeIntrinsic(CallBase *callInst);

} // namespace llvm::noelle