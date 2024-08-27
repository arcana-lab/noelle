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
  auto isPragma = [&](const Instruction *I, string funcName) -> bool {
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

  // A pragma is a CallInst whose first argument is global constant that
  // represents a string that starts with `directive`
  // The difference betweem a Begin and an End is the name of the called
  // function

  auto isBegin = bind(isPragma, placeholders::_1, "_Z19noelle_pragma_begin");
  auto isEnd = bind(isPragma, placeholders::_1, "_Z17noelle_pragma_end");

  this->MERT = new MultiExitRegionTree(F, isBegin, isEnd);
}

PragmaManager::~PragmaManager() {
  free(this->MERT);
}

MultiExitRegionTree *PragmaManager::getPragmaTree() {
  return this->MERT;
}

string PragmaManager::getRegionDirective(MultiExitRegionTree *T) const {
  auto CI = cast<CallInst>(T->getBegin());
  auto GEP = cast<GetElementPtrInst>(CI->getArgOperand(0));
  auto Ptr = GEP->getPointerOperand();
  auto GV = cast<GlobalVariable>(Ptr);
  auto CDA = cast<ConstantDataArray>(GV->getInitializer());
  return CDA->getAsCString().str();
}

vector<Value *> PragmaManager::getRegionArguments(
    MultiExitRegionTree *T) const {
  auto CI = cast<CallInst>(T->getBegin());

  // The first argument is skipped because it's the directive

  vector<Value *> args;
  for (size_t i = 1; i < CI->arg_size(); i++) {
    args.push_back(CI->getArgOperand(i));
  }

  return args;
}

raw_ostream &PragmaManager::print(raw_ostream &stream, string prefixToUse) {
  return this->print(this->MERT, stream, prefixToUse, LAST);
}

raw_ostream &PragmaManager::print(MultiExitRegionTree *T,
                                  raw_ostream &stream,
                                  string prefixToUse,
                                  SiblingType ST) {
  string nodePrefix = "";
  string nodeText = "";

  if (T != this->MERT) {
    switch (ST) {
      case INNER:
        nodePrefix = "\u2523\u2501 ";
        break;
      case LAST:
        nodePrefix = "\u2517\u2501 ";
        break;
    }
    nodeText = this->getRegionDirective(T);
  } else {
    nodeText = "\e[1;32m" + this->F.getName().str() + "\e[0m";
  }

  stream << prefixToUse << nodePrefix << nodeText << "\n";

  if (T != this->MERT) {
    if (ST == LAST) {
      prefixToUse += "   ";
    } else {
      prefixToUse += "\u2503  ";
    }
  }

  auto children = T->getChildren();
  auto N = children.size();

  if (N == 0) {
    return stream;
  }

  for (size_t i = 0; i < N - 1; i++) {
    this->print(children[i], stream, prefixToUse, INNER);
  }
  this->print(children[N - 1], stream, prefixToUse, LAST);

  return stream;
}

} // namespace arcana::noelle
