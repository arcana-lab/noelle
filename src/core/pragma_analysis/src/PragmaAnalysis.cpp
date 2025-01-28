/*
 * Copyright 2023 - 2024  Federico Sossai
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publoopsh, distribute, sublicense, and/or sell copies
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
#include "arcana/noelle/core/PragmaAnalysis.hpp"
#include "arcana/noelle/core/PragmaForest.hpp"

namespace arcana::noelle {

PragmaAnalysis::PragmaAnalysis() : DependenceAnalysis("PragmaAnalysis") {
  this->funcPrefixes = { "noelle_pragma_", "_Z19noelle_pragma_begin" };
  return;
}

PragmaAnalysis::~PragmaAnalysis() {}

bool PragmaAnalysis::canThereBeAMemoryDataDependence(Instruction *fromInst,
                                                     Instruction *toInst) {
  return !(isPragma(fromInst) || isPragma(toInst));
}

bool PragmaAnalysis::canThereBeAMemoryDataDependence(Instruction *fromInst,
                                                     Instruction *toInst,
                                                     Function &function) {
  return !(isPragma(fromInst) || isPragma(toInst));
}

bool PragmaAnalysis::canThereBeAMemoryDataDependence(Instruction *fromInst,
                                                     Instruction *toInst,
                                                     LoopStructure &loop) {
  return !(isPragma(fromInst) || isPragma(toInst));
}

MemoryDataDependenceStrength PragmaAnalysis::
    isThereThisMemoryDataDependenceType(DataDependenceType t,
                                        Instruction *fromInst,
                                        Instruction *toInst) {
  if (isPragma(fromInst) || isPragma(toInst)) {
    return CANNOT_EXIST;
  }
  return MAY_EXIST;
}

MemoryDataDependenceStrength PragmaAnalysis::
    isThereThisMemoryDataDependenceType(DataDependenceType t,
                                        Instruction *fromInst,
                                        Instruction *toInst,
                                        Function &function) {
  if (isPragma(fromInst) || isPragma(toInst)) {
    return CANNOT_EXIST;
  }
  return MAY_EXIST;
}

MemoryDataDependenceStrength PragmaAnalysis::
    isThereThisMemoryDataDependenceType(DataDependenceType t,
                                        Instruction *fromInst,
                                        Instruction *toInst,
                                        LoopStructure &loop) {
  if (isPragma(fromInst) || isPragma(toInst)) {
    return CANNOT_EXIST;
  }
  return MAY_EXIST;
}

bool PragmaAnalysis::canThisDependenceBeLoopCarried(DGEdge<Value, Value> *dep,
                                                    LoopStructure &loop) {
  return !(isPragma(dep->getSrc()) || isPragma(dep->getDst()));
}

bool PragmaAnalysis::isPragma(Value *V) const {
  if (auto CI = dyn_cast<CallInst>(V)) {
    if (auto Callee = CI->getCalledFunction()) {
      auto fName = Callee->getName();
      for (auto pref : this->funcPrefixes) {
        if (fName.startswith(pref)) {
          return true;
        }
      }
    }
  }
  return false;
}

} // namespace arcana::noelle
