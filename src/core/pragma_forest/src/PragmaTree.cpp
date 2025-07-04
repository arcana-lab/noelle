/*
 * Copyright 2024 - Federico Sossai
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
#include <functional>
#include <iterator>
#include <queue>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"

#include "arcana/noelle/core/LoopStructure.hpp"
#include "arcana/noelle/core/PragmaForest.hpp"

using namespace std;
using namespace llvm;

namespace arcana::noelle {

PragmaTree::PragmaTree(Function *F,
                       DominatorTree *DT,
                       Instruction *Begin,
                       Instruction *End)
  : F(F),
    DT(DT),
    parent(nullptr),
    Begin(Begin),
    End(End) {}

PragmaTree::~PragmaTree() {
  for (auto T : this->children) {
    delete T;
  }
}

void PragmaTree::addChild(PragmaTree *T) {
  assert(std::find(std::begin(this->children), std::end(this->children), T)
         == std::end(this->children));

  this->children.push_back(T);
  T->parent = this;
}

bool PragmaTree::getStringFromArg(Value *arg, StringRef &result) {
  auto GEP = dyn_cast<GetElementPtrInst>(arg);
  Value *Ptr = nullptr;
  if (GEP == nullptr) {
    auto CE = dyn_cast<ConstantExpr>(arg);
    if (CE == nullptr || CE->getOpcode() != Instruction::GetElementPtr) {
      return false;
    }
    Ptr = CE->getOperand(0);
  } else {
    Ptr = GEP->getPointerOperand();
  }
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
  result = CDA->getAsCString();
  return true;
}

bool PragmaTree::getDoubleFromArg(llvm::Value *arg, double &result) {
  if (auto Float = dyn_cast<ConstantFP>(arg)) {
    result = Float->getValue().convertToDouble();
    return true;
  }
  return false;
}

bool PragmaTree::getIntFromArg(llvm::Value *arg, int &result) {
  if (auto Int = dyn_cast<ConstantInt>(arg)) {
    result = Int->getSExtValue();
    return true;
  }
  return false;
}

bool PragmaTree::isEmpty() const {
  return this->children.size() == 0;
}

bool PragmaTree::contains(const Instruction *I) {
  if (!this->DT->dominates(this->Begin, I)) {
    // Dominance is a necessary condition therefore we can immediately return if
    // not met
    return false;
  }
  return findInnermostPragmaFor(I) != nullptr;
}

bool PragmaTree::contains(const BasicBlock *BB) {
  // The tree contains `BB` iff it contains its first and last instructions
  auto FirstI = &*BB->begin();
  auto LastI = BB->getTerminator();
  return contains(FirstI) && contains(LastI);
}

bool PragmaTree::contains(const LoopStructure *LS) {
  // The tree contains `LS` iff it contains its header and all latches
  for (auto Latch : LS->getLatches()) {
    if (!contains(Latch)) {
      return false;
    }
  }
  return contains(LS->getHeader());
}

bool PragmaTree::strictlyContains(const Instruction *I) {
  return this == findInnermostPragmaFor(I);
}

bool PragmaTree::strictlyContains(const BasicBlock *BB) {
  return this == findInnermostPragmaFor(BB);
}

bool PragmaTree::strictlyContains(const LoopStructure *LS) {
  return this == findInnermostPragmaFor(LS);
}

PragmaTree *PragmaTree::findInnermostPragmaFor(const Instruction *I) {
  queue<PragmaTree *> worklist;
  PragmaTree *innermost = nullptr;

  auto search = [&](PragmaTree *T, auto) -> bool {
    auto BeginBB = T->Begin->getParent();
    auto EndBB = T->End->getParent();
    auto TargetBB = I->getParent();

    if (BeginBB == EndBB) {
      if (TargetBB != BeginBB) {
        return false; // `T` is not the innermost
      }
      if (T->Begin->comesBefore(I) && I->comesBefore(T->End)) {
        innermost = T; // `T` is the innermost
        return true;   // stop search
      }
      return false; // `T` is not the innermost
    }
    // This pragma is across different basic blocks
    if (TargetBB == BeginBB) {
      if (T->Begin->comesBefore(I)) {
        innermost = T; // `T` is the innermost
        return true;   // stop search
      }
      return false; // `T` is not the innermost
    }
    if (TargetBB == EndBB) {
      if (I->comesBefore(T->End)) {
        innermost = T; // `T` is the innermost
        return true;   // stop search
      }
      return false; // `T` is not the innermost
    }

    // At this point, if `T` contains `I` it must be in one basic block within
    // the pragma region
    auto BBs = T->getBasicBlocksWithin();
    // Forgive me father for I have used a const_cast
    if (BBs.find(const_cast<BasicBlock *>(TargetBB)) != BBs.end()) {
      innermost = T; // `T` is the innermost
      return true;   // stop search
    }
    return false; // `T` is not the innermost
  };

  visitPostOrder(search);

  return innermost;
}

PragmaTree *PragmaTree::findInnermostPragmaFor(const BasicBlock *BB) {
  if (!contains(BB)) {
    return nullptr;
  }
  auto FirstI = &*BB->begin();
  return findInnermostPragmaFor(FirstI);
}

PragmaTree *PragmaTree::findInnermostPragmaFor(const LoopStructure *LS) {
  if (!contains(LS)) {
    return nullptr;
  }
  return findInnermostPragmaFor(LS->getHeader());
}

PragmaTree *PragmaTree::getRoot() {
  auto T = this;
  while (T->parent != nullptr) {
    T = T->parent;
  }
  return T;
}

unordered_set<Instruction *> PragmaTree::getInstructionsWithin() {
  unordered_set<Instruction *> Is;

  auto BeginBB = this->Begin->getParent();
  auto EndBB = this->End->getParent();
  if (BeginBB == EndBB) {
    // Adding instructions in between `Begin` and `End`
    auto itB = this->Begin->getIterator();
    auto itE = this->End->getIterator();
    while (itB != itE) {
      Is.insert(&*itB);
      ++itB;
    }
    Is.insert(&*itE);
  }

  for (auto BB : getBasicBlocksWithin()) {
    for (auto &I : *BB) {
      Is.insert(&I);
    }
  }

  return Is;
}

unordered_set<BasicBlock *> PragmaTree::getBasicBlocksWithin() {
  auto BeginBB = this->Begin->getParent();
  auto EndBB = this->End->getParent();

  if (BeginBB == EndBB) {
    auto FirstI = &*BeginBB->begin();
    auto LastI = BeginBB->getTerminator();
    if (FirstI == this->Begin && LastI == this->End) {
      return { BeginBB };
    }
    return {};
  }

  // Upward breadth-first search on the CFG starting from the predecessors of
  // `EndBB`.

  queue<BasicBlock *> worklist;
  unordered_set<BasicBlock *> enqueued;
  unordered_set<BasicBlock *> BBs;

  for (auto predBB : predecessors(EndBB)) {
    worklist.push(predBB);
    enqueued.insert(predBB);
  }

  while (!worklist.empty()) {
    auto BB = worklist.front();
    worklist.pop();

    if (!this->DT->dominates(this->Begin, BB)) {
      continue;
    }
    BBs.insert(BB);

    for (auto predBB : predecessors(BB)) {
      bool notEnqueued = enqueued.find(predBB) == enqueued.end();
      if (notEnqueued) {
        worklist.push(predBB);
        enqueued.insert(predBB);
      }
    }
  }

  return BBs;
}

vector<PragmaTree *> PragmaTree::getAncestorsFromRootTo(
    const llvm::Instruction *I) {
  auto destination = findInnermostPragmaFor(I);
  if (destination == nullptr) {
    return {};
  }

  // Upward tree traversal
  stack<PragmaTree *> ancestors;
  auto current = destination;
  while (current != this) {
    ancestors.push(current);
    current = current->parent;
  }
  ancestors.push(this);

  vector<PragmaTree *> path;
  while (!ancestors.empty()) {
    path.push_back(ancestors.top());
    ancestors.pop();
  }
  return path;
}

vector<PragmaTree *> PragmaTree::getChildren() const {
  return this->children;
}

Instruction *PragmaTree::getBeginDelimiter() const {
  return this->Begin;
}

Instruction *PragmaTree::getEndDelimiter() const {
  return this->End;
}

PragmaTree *PragmaTree::getParent() {
  return this->parent;
}

string PragmaTree::getDirective() const {
  auto CI = cast<CallInst>(this->Begin);
  auto FirstArg = CI->getArgOperand(0);
  auto GEP = dyn_cast<GetElementPtrInst>(FirstArg);

  Value *Ptr = nullptr;
  if (GEP == nullptr) {
    auto CE = cast<ConstantExpr>(FirstArg);
    Ptr = CE->getOperand(0);
  } else {
    Ptr = GEP->getPointerOperand();
  }

  auto GV = cast<GlobalVariable>(Ptr);
  auto CDA = cast<ConstantDataArray>(GV->getInitializer());
  return CDA->getAsCString().str();
}

vector<Value *> PragmaTree::getArguments() const {
  auto CI = cast<CallInst>(this->Begin);
  vector<Value *> args;

  // The first argument is skipped because it's the directive.
  // The rest is just the list of args itself
  for (size_t i = 1; i < CI->arg_size(); i++) {
    args.push_back(CI->getArgOperand(i));
  }
  return args;
}

raw_ostream &PragmaTree::print(raw_ostream &stream, string prefix) {
  return print(stream, prefix, LAST);
}

raw_ostream &PragmaTree::print(raw_ostream &stream,
                               string prefix,
                               SiblingType ST) {
  string nodePrefix = "";

  switch (ST) {
    case INNER:
      nodePrefix = "\u2523\u2501 ";
      break;
    case LAST:
      nodePrefix = "\u2517\u2501 ";
      break;
  }

  // Printing arguments

  stream << prefix << nodePrefix << getDirective() << "";
  auto Args = getArguments();
  if (Args.size() > 0) {
    stream << " (";
  }
  for (size_t i = 0; i < Args.size(); i++) {
    auto &A = Args[i];
    StringRef valString;
    int valInt;
    double valDouble;
    bool isString = getStringFromArg(A, valString);
    bool isInt = getIntFromArg(A, valInt);
    bool isDouble = getDoubleFromArg(A, valDouble);
    if (isString) {
      stream << "\"" << valString << "\"";
    } else if (isInt) {
      stream << valInt;
    } else if (isDouble) {
      stream << valDouble;
    } else {
      stream << "ptr*";
    }
    if (i != Args.size() - 1) {
      stream << ", ";
    }
  }
  if (Args.size() > 0) {
    stream << ")";
  }
  stream << "\n";

  if (ST == LAST) {
    prefix += "   ";
  } else {
    prefix += "\u2503  ";
  }

  auto N = this->children.size();

  if (N >= 1) {
    for (size_t i = 0; i < N - 1; i++) {
      this->children[i]->print(stream, prefix, INNER);
    }
    this->children[N - 1]->print(stream, prefix, LAST);
  }

  return stream;
}

void PragmaTree::erase() {
  visitPostOrder([&](PragmaTree *T, auto) -> bool {
    T->Begin->eraseFromParent();
    T->Begin = nullptr;
    T->End->eraseFromParent();
    T->End = nullptr;
    return false;
  });

  if (this->parent != nullptr) {
    auto &siblings = this->parent->children;
    auto thisIt = std::find(siblings.begin(), siblings.end(), this);
    assert(thisIt != siblings.end());
    siblings.erase(thisIt);
  }
  for (auto T : this->children) {
    delete T;
  }
  this->children.clear();
}

bool PragmaTree::visitPreOrder(
    std::function<bool(PragmaTree *T, uint32_t level)> callback) {
  return this->visitPreOrder(callback, 1);
}

bool PragmaTree::visitPostOrder(
    std::function<bool(PragmaTree *T, uint32_t level)> callback) {
  return this->visitPostOrder(callback, 1);
}

bool PragmaTree::visitPreOrder(
    std::function<bool(PragmaTree *T, uint32_t level)> callback,
    uint32_t level) {
  if (callback(this, level)) {
    return true;
  }
  for (auto child : this->children) {
    if (child->visitPreOrder(callback, level + 1)) {
      return true;
    }
  }
  return false;
}

bool PragmaTree::visitPostOrder(
    std::function<bool(PragmaTree *T, uint32_t level)> callback,
    uint32_t level) {
  for (auto child : this->children) {
    if (child->visitPostOrder(callback, level + 1)) {
      return true;
    }
  }
  if (callback(this, level)) {
    return true;
  }
  return false;
}

} // namespace arcana::noelle
