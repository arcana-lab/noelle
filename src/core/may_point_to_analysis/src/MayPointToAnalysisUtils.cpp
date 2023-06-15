#include "noelle/core/MayPointToAnalysis.hpp"
#include "MayPointToAnalysisUtils.hpp"

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

string getCalledFuncName(CallBase *callInst) {
  auto calledFunc = callInst->getCalledFunction();
  if (!calledFunc)
    return "";
  return calledFunc->getName();
}

PointToGraph add(const PointToGraph &lhs, const PointToGraph &rhs) {
  PointToGraph result = lhs;
  result.insert(rhs.begin(), rhs.end());
  return result;
}

PointToGraph minus(const PointToGraph &lhs, const PointToGraph &rhs) {
  PointToGraph result = lhs;
  for (auto pair : rhs) {
    result.erase(pair.first);
  }
  return result;
}

bool isLifetimeIntrinsic(CallBase *callInst) {
  auto intrinsic = dyn_cast<IntrinsicInst>(callInst);
  if (!intrinsic) {
    return false;
  }
  return intrinsic->isLifetimeStartOrEnd();
}

} // namespace llvm::noelle