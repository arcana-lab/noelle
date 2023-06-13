#pragma once

#include "noelle/core/MayPointToAnalysis.hpp"

#include <unordered_set>
#include <set>

namespace llvm::noelle {

Value *strip(Value *pointer);

MemoryObjects intersect(const MemoryObjects &lhs, const MemoryObjects &rhs);

MemoryObjects unite(const MemoryObjects &lhs, const MemoryObjects &rhs);

MemoryObjects minus(const MemoryObjects &lhs, const MemoryObjects &rhs);

MemoryObjects replace(const MemoryObjects &memObjSet,
                      MemoryObject *oldObj,
                      MemoryObject *newObj);

std::string getCalledFuncName(llvm::CallInst *callInst);

PointToGraph add(const PointToGraph &lhs, const PointToGraph &rhs);

PointToGraph minus(const PointToGraph &lhs, const PointToGraph &rhs);

} // namespace llvm::noelle