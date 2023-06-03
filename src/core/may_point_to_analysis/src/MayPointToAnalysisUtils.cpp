
#include "noelle/core/MayPointToAnalysisUtils.hpp"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/Casting.h"

#include <unordered_set>
#include <unordered_map>
#include <set>

using namespace std;

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

MemoryObjects replace(const MemoryObjects &memObjSet,
                      MemoryObject *oldObj,
                      MemoryObject *newObj) {
  MemoryObjects result(memObjSet);
  result.erase(oldObj);
  result.insert(newObj);
  return result;
}

string getCalledFuncName(llvm::CallInst *callInst) {
  auto calledFunc = callInst->getCalledFunction();
  if (!calledFunc)
    return "";
  return calledFunc->getName();
}

PointToInfo add(const PointToInfo &lhs, const PointToInfo &rhs) {
  PointToInfo result = lhs;
  result.insert(rhs.begin(), rhs.end());
  return result;
}

PointToInfo minus(const PointToInfo &lhs, const PointToInfo &rhs) {
  PointToInfo result = lhs;
  for (auto pair : rhs) {
    result.erase(pair.first);
  }
  return result;
}

} // namespace llvm::noelle