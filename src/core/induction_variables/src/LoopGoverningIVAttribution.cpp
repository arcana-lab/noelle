/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/LoopGoverningIVAttribution.hpp"

namespace llvm::noelle {

  LoopGoverningIVAttribution::LoopGoverningIVAttribution (InductionVariable &iv, SCC &scc, std::vector<BasicBlock *> &exitBlocks)
    : 
      IV{iv}
  , scc{scc}
  , headerCmp{nullptr}
  , conditionValueDerivation{}
  , intermediateValueUsedInCompare{nullptr}
  , isWellFormed{false} 
  {

    /*
     * To understand how to transform the loop governing condition, it is far simpler to
     * know the sign of the step size at compile time. Extra overhead is necessary if this
     * is only known at runtime, and that enhancement has yet to be made
     */
    if (!iv.getSingleComputedStepValue() || !(isa<ConstantInt>(iv.getSingleComputedStepValue()) || isa<ConstantFP>(iv.getSingleComputedStepValue()))) {
      return;
    }

    auto headerPHI = iv.getLoopEntryPHI();
    auto ivInstructions = iv.getAllInstructions();

    /*
     * This attribution only understands integer and floating point typed induction variables
     */
    if (!headerPHI->getType()->isIntegerTy() && !headerPHI->getType()->isFloatingPointTy()) {
      return;
    }

    /*
     * Fetch the loop governing terminator.
     * NOTE: It should be the only conditional branch in the IV's SCC
     */
    BranchInst *loopGoverningTerminator = nullptr;
    auto sccOfIV = iv.getSCC();
    for (auto internalNodePair : sccOfIV->internalNodePairs()) {

      /*
       * Fetch the current instruction within the SCC that contains the IV
       */
      auto value = internalNodePair.first;

      /*
       * Check whether the current instruction is a branch
       */
      if (!isa<BranchInst>(value)) {
        continue;
      }

      /*
       * The current instruction is a branch.
       * Check whether it is a conditional one.
       */
      auto br = cast<BranchInst>(value);
      if (!br->isConditional()) {
        continue;
      }

      /*
       * The current branch is a conditional one.
       */
      if (loopGoverningTerminator) {
        return;
      }
      loopGoverningTerminator = br;
    }

    /*
     * Ensure the branch is in the header as this analysis does not understand do-while loops
     */
    if (!loopGoverningTerminator) {

      /*
       * This is a do-while loop.
       * We assume the loop has been normalized it to a while shape.
       * We exit here as we do not support do-while loops.
       */
      return;
    }
    if (loopGoverningTerminator->getParent() != headerPHI->getParent()) {

      /*
       * This is a do-while loop.
       * We assume the loop has been normalized it to a while shape.
       * We exit here as we do not support do-while loops.
       */
      return;
    }

    /*
     * The loop is in a while shape.
     */
    this->headerBr = loopGoverningTerminator;

    /*
     * Fetch the condition of the conditional branch
     */
    auto headerCondition = headerBr->getCondition();
    if (!isa<CmpInst>(headerCondition)) {
      return;
    }

    /*
     * Find the last value and the updated value of the IV.
     */
    this->headerCmp = cast<CmpInst>(headerCondition);
    auto opL = headerCmp->getOperand(0), opR = headerCmp->getOperand(1);
    auto isOpLHSLoopEntryPHI = isa<Instruction>(opL) && headerPHI == cast<Instruction>(opL);
    auto isOpRHSLoopEntryPHI = isa<Instruction>(opR) && headerPHI == cast<Instruction>(opR);
    if (!(isOpLHSLoopEntryPHI ^ isOpRHSLoopEntryPHI)) {
      
      /*
       * The value of the IV used to compare against the condition value is not the PHI, but it is the updated value.
       *
       * Find the updated value used in the compare.
       */
      for (auto intermediateValue : iv.getNonPHIIntermediateValues()){
        if (  false
              || (intermediateValue == opR)
              || (intermediateValue == opL)
           ){
          this->intermediateValueUsedInCompare = intermediateValue;
          break ;
        }
      }
      if (this->intermediateValueUsedInCompare == nullptr){
        return ;
      }
      if (this->intermediateValueUsedInCompare == opR){
        this->conditionValue = opL;
      } else {
        this->conditionValue = opR;
      }

    } else {
      this->conditionValue = isOpLHSLoopEntryPHI ? opR : opL;
      this->intermediateValueUsedInCompare = cast<Instruction>(isOpLHSLoopEntryPHI ? opL : opR);
    }

    /*
     * Find the single exit basic block.
     */
    std::set<BasicBlock *> exitBlockSet(exitBlocks.begin(), exitBlocks.end());
    if (exitBlockSet.find(headerBr->getSuccessor(0)) != exitBlockSet.end()) {
      this->exitBlock = headerBr->getSuccessor(0);
    } else if (exitBlockSet.find(headerBr->getSuccessor(1)) != exitBlockSet.end()) {
      this->exitBlock = headerBr->getSuccessor(1);
    } else {
      return ;
    }

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
          if (!edge->isDataDependence()) {
            continue;
          }

          auto outgoingValue = edge->getOutgoingT();
          if (!scc.isInternal(outgoingValue)) {
            continue ;
          }
          assert(isa<Instruction>(outgoingValue)
              && "An internal value to an IV's SCC must be an instruction!");
          auto outgoingInst = cast<Instruction>(outgoingValue);

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

    isWellFormed = true;

    return ;
  }

  InductionVariable &LoopGoverningIVAttribution::getInductionVariable(void) const {
    return IV;
  }

  CmpInst * LoopGoverningIVAttribution::getHeaderCompareInstructionToComputeExitCondition(void) const {
    return headerCmp;
  }

  Value * LoopGoverningIVAttribution::getExitConditionValue (void) const {
    return conditionValue;
  }

  BranchInst * LoopGoverningIVAttribution::getHeaderBrInst(void) const {
    return headerBr;
  }

  BasicBlock * LoopGoverningIVAttribution::getExitBlockFromHeader(void) const {
    return exitBlock;
  }

  bool LoopGoverningIVAttribution::isSCCContainingIVWellFormed(void) const {
    return isWellFormed;
  }

  std::set<Instruction *> LoopGoverningIVAttribution::getConditionValueDerivation(void) const { 
    return conditionValueDerivation;
  }

  Instruction * LoopGoverningIVAttribution::getValueToCompareAgainstExitConditionValue (void) const {
    return intermediateValueUsedInCompare;
  }

}
