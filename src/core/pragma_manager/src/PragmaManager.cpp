/*
 * Copyright 2021 - 2024 Federico Sossai, Simone Campanoni
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

#include <algorithm>
#include <cmath>
#include <functional>
#include <stack>
#include <string>

#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"

#include "arcana/noelle/core/MultiExitRegionTree.hpp"
#include "arcana/noelle/core/PragmaManager.hpp"

using namespace std;
using namespace llvm;

namespace arcana::noelle {

PragmaManager::PragmaManager(Function &F, string directive)
  : F(F),
    directive(directive) {
  auto matcher = [&](const Instruction *I, string funcName) -> bool {
    auto CI = dyn_cast<CallInst>(I);
    if (CI == nullptr) {
      return false;
    }
    auto callee = CI->getCalledFunction();
    if (callee == nullptr) {
      return false;
    }
    if (!callee->getName().startswith(funcName)) {
      return false;
    }
    auto GEP = dyn_cast<GetElementPtrInst>(CI->getArgOperand(0));
    if (GEP == nullptr) {
      return false;
    }
    auto Ptr = GEP->getPointerOperand();
    if (!isa<Constant>(Ptr)) {
      return false;
    }
    auto GV = dyn_cast<llvm::GlobalVariable>(Ptr);
    if (GV == nullptr || !GV->isConstant()) {
      return false;
    }
    auto ATy = dyn_cast<llvm::ArrayType>(GV->getValueType());
    if (ATy == nullptr) {
      return false;
    }
    if (!ATy->getElementType()->isIntegerTy(8)) {
      return false;
    }
    auto *CDA = llvm::dyn_cast<llvm::ConstantDataArray>(GV->getInitializer());
    if (!CDA->isString()) {
      return false;
    }
    auto CString = CDA->getAsCString().str();
    if (!CDA->getAsCString().startswith(directive)) {
      return false;
    }
    return true;
  };

  auto isBegin = bind(matcher, placeholders::_1, "_Z19noelle_pragma_begin");
  auto isEnd = bind(matcher, placeholders::_1, "_Z17noelle_pragma_end");

  this->MERT = new MultiExitRegionTree(F, isBegin, isEnd);
}

PragmaManager::~PragmaManager() {
  free(this->MERT);
}

MultiExitRegionTree *PragmaManager::getPragmaTree() {
  return this->MERT;
}

string PragmaManager::getPragmaTreeName(MultiExitRegionTree *T) const {
  auto CI = cast<CallInst>(T->getBegin());
  auto GEP = cast<GetElementPtrInst>(CI->getArgOperand(0));
  auto Ptr = GEP->getPointerOperand();
  auto GV = cast<GlobalVariable>(Ptr);
  auto CDA = cast<ConstantDataArray>(GV->getInitializer());
  return CDA->getAsCString().str();
}

vector<Value *> PragmaManager::getPragmaTreeArguments(
    MultiExitRegionTree *T) const {
  auto CI = cast<CallInst>(T->getBegin());

  vector<Value *> args;
  for (size_t i = 1; i < CI->arg_size(); i++) {
    args.push_back(CI->getArgOperand(i));
  }

  return args;
}

raw_ostream &PragmaManager::print(raw_ostream &stream,
                                  string prefixToUse,
                                  bool printArguments) {
  stack<MultiExitRegionTree *> worklist;
  stack<int> levels;
  string symbol = "\u2500";

  auto getLevelPrefix = [&](int level) { return string(3 * level, ' '); };

  auto CC = this->MERT->getChildren();
  for (auto it = CC.rbegin(); it != CC.rend(); ++it) {
    worklist.push(*it);
    levels.push(0);
  }

  while (!worklist.empty()) {
    auto T = worklist.top();
    auto level = levels.top();
    worklist.pop();
    levels.pop();
    auto directive = this->getPragmaTreeName(T);
    auto levelPrefix = getLevelPrefix(level);

    stream << prefixToUse << levelPrefix << directive;
    if (printArguments) {
      stream << " ( ";
      for (auto A : this->getPragmaTreeArguments(T)) {
        stream << *A << "; ";
      }
      stream << ")";
    }
    stream << "\n";

    auto CC = T->getChildren();
    for (auto it = CC.rbegin(); it != CC.rend(); ++it) {
      worklist.push(*it);
      levels.push(level + 1);
    }
  }

  return stream;
}

} // namespace arcana::noelle
