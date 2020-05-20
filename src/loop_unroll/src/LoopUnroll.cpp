#include "LoopUnroll.hpp"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

using namespace llvm;
 
bool LoopUnroll::fullyUnrollLoop (
  LoopDependenceInfo const &LDI
  ){
  auto modified = false;

  /*
   * Fetch the loop summary
   */
  auto ls = LDI.getLoopSummary();


  return modified;
}
