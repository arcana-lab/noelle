/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopGoverningIVAttribution.hpp"

using namespace llvm;

LoopGoverningIVAttribution::LoopGoverningIVAttribution (InductionVariable &iv, SCC &scc, std::vector<BasicBlock *> &exitBlocks)
  : IV{iv}, scc{scc}, headerCmp{nullptr}, conditionValueDerivation{},
    intermediateValueUsedInCompare{nullptr}, isWellFormed{false} {

  iv.getHeaderPHI()->print(errs() << "Checking for loop governance: "); errs() << "\n";

  /*
   * To understand how to transform the loop governing condition, it is far simpler to
   * know the sign of the step size at compile time. Extra overhead is necessary if this
   * is only known at runtime, and that enhancement has yet to be made
   */
  if (!iv.getSimpleValueOfStepSize() || !isa<ConstantInt>(iv.getSimpleValueOfStepSize())) return;
  iv.getHeaderPHI()->print(errs() << "Has step size: "); errs() << "\n";

  auto headerPHI = iv.getHeaderPHI();
  auto &ivInstructions = iv.getAllInstructions();

  /*
   * Fetch the loop governing terminator.
   * NOTE: It should be the only conditional branch in the IV's SCC
   */
  BranchInst *loopGoverningTerminator = nullptr;
  for (auto node : iv.getSCC()->getNodes()) {
    auto value = node->getT();
    if (!isa<BranchInst>(value)) continue;
    auto br = cast<BranchInst>(value);
    if (!br->isConditional()) continue;
    if (loopGoverningTerminator) return;
    loopGoverningTerminator = br;
  }
  if (!loopGoverningTerminator) return;
  this->headerBr = loopGoverningTerminator;

  this->headerBr->print(errs() << "Has branch: "); errs() << "\n";

  /*
   * Fetch the condition of the conditional branch
   */
  auto headerCondition = headerBr->getCondition();
  if (!isa<CmpInst>(headerCondition)) return;

  this->headerCmp = cast<CmpInst>(headerCondition);
  auto opL = headerCmp->getOperand(0), opR = headerCmp->getOperand(1);
  auto isOpLHSAnIntermediate = isa<Instruction>(opL)
    && ivInstructions.find(cast<Instruction>(opL)) != ivInstructions.end();
  auto isOpRHSAnIntermediate = isa<Instruction>(opR)
    && ivInstructions.find(cast<Instruction>(opR)) != ivInstructions.end();
  for (auto I : ivInstructions) { I->print(errs() << "IV INST: "); errs() << "\n"; }
  if (!(isOpLHSAnIntermediate ^ isOpRHSAnIntermediate)) return;
  this->conditionValue = isOpLHSAnIntermediate ? opR : opL;
  this->intermediateValueUsedInCompare = cast<Instruction>(isOpLHSAnIntermediate ? opL : opR);

  this->headerCmp->print(errs() << "Has comparison: "); errs() << "\n";
  this->conditionValue->print(errs() << "Has condition: "); errs() << "\n";

  std::set<BasicBlock *> exitBlockSet(exitBlocks.begin(), exitBlocks.end());
  if (exitBlockSet.find(headerBr->getSuccessor(0)) != exitBlockSet.end()) {
    this->exitBlock = headerBr->getSuccessor(0);
  } else if (exitBlockSet.find(headerBr->getSuccessor(1)) != exitBlockSet.end()) {
    this->exitBlock = headerBr->getSuccessor(1);
  } else return ;

  iv.getHeaderPHI()->print(errs() << "Has one exit: "); errs() << "\n";

  if (scc.isInternal(conditionValue)) {
    std::queue<Instruction *> conditionDerivation;
    assert(isa<Instruction>(conditionValue)
      && "An internal value to an IV's SCC must be an instruction!");
    conditionDerivation.push(cast<Instruction>(conditionValue));

    while (!conditionDerivation.empty()) {
      auto value = conditionDerivation.front();
      conditionDerivation.pop();

      auto valueNodeInSCC = scc.fetchNode(value);
      for (auto edge : valueNodeInSCC->getIncomingEdges()) {
        if (!edge->isDataDependence()) continue;

        auto outgoingValue = edge->getOutgoingT();
        if (!scc.isInternal(outgoingValue)) {
          continue ;
        }
        assert(isa<Instruction>(outgoingValue)
          && "An internal value to an IV's SCC must be an instruction!");
        auto outgoingInst = cast<Instruction>(outgoingValue);

        outgoingInst->print(errs() << "Exit condition depends on: "); errs() << "\n";

        /*
         * The exit condition value cannot be itself derived from the induction variable 
         */
        if (ivInstructions.find(outgoingInst) != ivInstructions.end()) {
          return;
        }

        /*
         * Check if we have already considered outgoingInst.
         */
        if (this->conditionValueDerivation.find(outgoingInst) != this->conditionValueDerivation.end()){
          continue ;
        }

        /*
         * @outgoingInst hasn't been considered yet.
         * It's time to consider it.
         */
        conditionValueDerivation.insert(outgoingInst);
        conditionDerivation.push(outgoingInst);
      }
    }
  }

  iv.getHeaderPHI()->print(errs() << "Is well formed: "); errs() << "\n";

  isWellFormed = true;
}

InductionVariable &LoopGoverningIVAttribution::getInductionVariable(void) const {
  return IV;
}

CmpInst *LoopGoverningIVAttribution::getHeaderCmpInst(void) const {
  return headerCmp;
}

Value *LoopGoverningIVAttribution::getHeaderCmpInstConditionValue(void) const {
  return conditionValue;
}

BranchInst *LoopGoverningIVAttribution::getHeaderBrInst(void) const {
  return headerBr;
}

BasicBlock *LoopGoverningIVAttribution::getExitBlockFromHeader(void) const {
  return exitBlock;
}

bool LoopGoverningIVAttribution::isSCCContainingIVWellFormed(void) const {
  return isWellFormed;
}

std::set<Instruction *> &LoopGoverningIVAttribution::getConditionValueDerivation(void) { 
  return conditionValueDerivation;
}

Instruction *LoopGoverningIVAttribution::getIntermediateValueUsedInCompare () {
  return intermediateValueUsedInCompare;
}
