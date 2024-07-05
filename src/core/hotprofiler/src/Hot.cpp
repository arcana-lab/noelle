/*
 * Copyright 2016 - 2024  Angelo Matni, Simone Campanoni
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
#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/Hot.hpp"

namespace arcana::noelle {

Hot::Hot(Module &M,
         std::function<llvm::BlockFrequencyInfo &(Function &F)> getBFI,
         std::function<llvm::BranchProbabilityInfo &(Function &F)> getBPI)
  : moduleNumberOfInstructionsExecuted{ 0 },
    getBFI{ getBFI },
    getBPI{ getBPI } {

  /*
   * Analyze the profiles.
   */
  this->analyzeProfiles(M);

  return;
}

void Hot::analyzeProfiles(Module &M) {

  /*
   * Fetch the invocations of each basic block of each function.
   */
  for (auto &F : M) {
    if (F.empty()) {
      continue;
    }
    auto &bfi = this->getBFI(F);
    auto &bpi = this->getBPI(F);

    /*
     * Set the invocations of basic blocks.
     */
    for (auto &bb : F) {

      /*
       * Check if the basic block has been executed at least once.
       */
      auto count = bfi.getBlockProfileCount(&bb);
      if (!count){

        /*
         * The basic block hasn't been executed.
         */
        this->setBasicBlockInvocations(&bb, 0);
        continue;
      }

      /*
       * Fetch the basic block counter.
       */
      auto v = *count;

      /*
       * Set the invocations.
       */
      this->setBasicBlockInvocations(&bb, v);

      /*
       * Compute the frequency of jumping to the successors of bb.
       */
      for (auto succBB : successors(&bb)) {
        auto prob = bpi.getEdgeProbability(&bb, succBB);
        if (prob.isUnknown()) {
          continue;
        }
        auto probNum = double(prob.getNumerator());
        auto probDen = double(prob.getDenominator());
        auto probValue = probNum / probDen;

        /*
         * Set the frequency.
         */
        this->setBranchFrequency(&bb, succBB, probValue);
      }
    }
  }

  /*
   * Compute the global counters.
   */
  this->computeProgramInvocations(M);

  return;
}

bool Hot::isAvailable(void) const {
  return this->hasBeenExecuted();
}

void Hot::computeProgramInvocations(Module &M) {

  /*
   * Compute the total number of instructions executed.
   */
  for (auto pairs : this->bbInvocations) {

    /*
     * Fetch the current basic block
     */
    auto bb = pairs.first;

    /*
     * Fetch the number of invocations of the basic block and its length.
     */
    auto totalBBInsts = this->getInvocations(bb);
    auto bbLength = this->getStaticInstructions(bb);

    /*
     * Update the module counter
     */
    this->moduleNumberOfInstructionsExecuted += (totalBBInsts * bbLength);
  }

  /*
   * Compute the total number of instructions executed by each function.
   * Each call instructions is considered one; so callee instructions are not
   * considered.
   */
  for (auto pairs : this->functionInvocations) {

    /*
     * Fetch the function.
     */
    auto f = pairs.first;

    /*
     * Consider all basic blocks.
     */
    uint64_t c = 0;
    for (auto &bb : *f) {
      auto totalBBInsts =
          (this->getInvocations(&bb) * this->getStaticInstructions(&bb));
      c += totalBBInsts;
    }
    this->functionSelfInstructions[f] = c;
  }

  /*
   * Compute the total instructions.
   */
  this->computeTotalInstructions(M);

  return;
}

void Hot::computeTotalInstructions(Module &moduleToAnalyze) {

  /*
   * Analyze every function included in M and compute their total instructions
   * executed.
   *
   * To do so, we iterate over all functions of M.
   */
  for (auto &F : moduleToAnalyze) {

    /*
     * Fetch the next function defined within the module.
     */
    if (F.empty()) {
      continue;
    }

    /*
     * Compute the total instructions of F.
     */
    std::unordered_map<Function *, bool> evaluationStack;
    this->computeTotalInstructions(F, evaluationStack);
  }

  /*
   * Analyze every call instruction.
   *
   * To do so, we iterate over all functions and check all of their callers.
   */
  for (auto &F : moduleToAnalyze) {

    /*
     * Fetch the next function defined within the module.
     */
    if (F.empty()) {
      continue;
    }

    /*
     * Check if the function has been executed at all.
     */
    if (!this->hasBeenExecuted(&F)) {
      continue;
    }

    /*
     * Fetch the total instructions executed by this function.
     */
    assert(this->isFunctionTotalInstructionsAvailable(F));
    auto totalInstsOfF = this->getTotalInstructions(&F);
    auto totalInstsOfFPerInvocation = totalInstsOfF / this->getInvocations(&F);
    auto totalInstsOfFLeftover = totalInstsOfF;

    /*
     * Fetch all callers of the function.
     */
    Instruction *callerOfF = nullptr;
    for (auto &useOfF : F.uses()) {

      /*
       * Fetch the next call instruction to F.
       */
      auto userOfF = useOfF.getUser();
      if (!isa<CallBase>(userOfF)) {
        continue;
      }
      callerOfF = cast<Instruction>(userOfF);

      /*
       * The instruction "userOfF" invokes F.
       *
       * Check if the caller has been executed at all.
       */
      if (!this->hasBeenExecuted(callerOfF)) {
        continue;
      }

      /*
       * Compute the fraction of the callee that is associated to the specific
       * call instruction we are analyzying. To this end, we make the assumption
       * that the distribution of total instructions of the callee is uniform
       * among its invocations.
       *
       * The total number of instructions executed by this call is the call
       * itself plus the total instructions executed by the callee divided by
       * its invocations due to this call.
       */
      auto instructionInvocations = this->getInvocations(callerOfF);
      auto calleeTotalInstsFraction =
          totalInstsOfFPerInvocation * instructionInvocations;
      this->instructionTotalInstructions[callerOfF] =
          calleeTotalInstsFraction + 1;

      /*
       * Remove the current fraction used of the callee.
       * This is needed to make sure the whole set of instructions of the callee
       * will be distributed among the callers.
       */
      totalInstsOfFLeftover -= calleeTotalInstsFraction;
    }

    /*
     * Check if all the total instructions have been distributed among the
     * callers.
     */
    if (true && (totalInstsOfFLeftover > 0)
        && (callerOfF != nullptr) /* This handle the case where a function has
                                     not known callers  */
    ) {

      /*
       * There is some leftover to still distribute.
       * This happen when the total instructions of the callee is not a multiple
       * of the number of the callers.
       *
       * Our heuristic is to give the leftover to the last caller (this is
       * completely arbitrary).
       */
      this->instructionTotalInstructions[callerOfF] += totalInstsOfFLeftover;
    }
  }

  return;
}

void Hot::computeTotalInstructions(
    Function &F,
    std::unordered_map<Function *, bool> &evaluationStack) {

  /*
   * Keep track we are evaluating the input function.
   */
  evaluationStack[&F] = true;

  /*
   * Check if the callee has been executed at all.
   */
  if (!this->hasBeenExecuted(&F)) {
    this->setFunctionTotalInstructions(&F, 0);
    return;
  }

  /*
   * Iterate over all instructions.
   */
  uint64_t t = 0;
  for (auto &inst : instructions(&F)) {

    /*
     * Check if the instruction has been executed at all.
     */
    if (!this->hasBeenExecuted(&inst)) {
      continue;
    }

    /*
     * Fetch the number of invocations of this instruction.
     */
    auto instructionInvocations = this->getInvocations(&inst);

    /*
     * Count the instruction.
     *
     * Notice that this needs to be done even for call instructions.
     */
    t += instructionInvocations;

    /*
     * Check if the instruction can invoke another function.
     */
    if (!isa<CallBase>(&inst)) {
      continue;
    }
    auto callInst = dyn_cast<CallBase>(&inst);

    /*
     * The instruction invokes another function.
     * Hence, we need to count the instructions executed by the callee.
     *
     * Fetch the callee.
     */
    auto callee = callInst->getCalledFunction();

    /*
     * Check if the callee is known and we can inspect its body.
     */
    if ((callee == nullptr) || (callee->empty())) {
      continue;
    }
    assert(this->hasBeenExecuted(callee));
    assert(this->getInvocations(callee) >= this->getInvocations(callInst));

    /*
     * The callee is known and we can inspect its body.
     *
     * Check if we have already analyzed the callee.
     */
    uint64_t calleeTotalInsts = 0;
    if (!this->isFunctionTotalInstructionsAvailable(*callee)) {

      /*
       * We have not analyzed the callee.
       *
       * Check if we are in the process of evaluating it.
       */
      if (evaluationStack[callee]) {

        /*
         * We are in the middle of evaluating the callee.
         * We have to break this evaluation cycle to converge.
         * For now, we consider the cost of the callee to be 1.
         */
        calleeTotalInsts = 1;

      } else {

        /*
         * We are not in the middle of the process of evaluating the callee.
         *
         * It is time to evaluate it.
         */
        this->computeTotalInstructions(*callee, evaluationStack);
        assert(this->isFunctionTotalInstructionsAvailable(*callee));

        /*
         * Now that the callee has been evaluated, we can fetch that
         * information.
         */
        calleeTotalInsts = this->getTotalInstructions(callee);
      }

    } else {

      /*
       * The callee has been evalauted already.
       */
      calleeTotalInsts = this->getTotalInstructions(callee);
    }
    assert(calleeTotalInsts > 0);

    /*
     * Compute the fraction of the callee that is associated to the specific
     * call instruction we are analyzying. To this end, we make the assumption
     * that the distribution of total instructions per callee invocation is
     * uniform among its dynamic callers.
     */
    auto calleeTotalInstsPerInvocationOfCalleeTmp =
        ((double)calleeTotalInsts) / ((double)this->getInvocations(callee));
    auto calleeTotalInstsPerInvocationOfCallee =
        (uint64_t)calleeTotalInstsPerInvocationOfCalleeTmp;

    /*
     * Add the fraction of the total of the callee associated to the current
     * instruction.
     */
    t += (calleeTotalInstsPerInvocationOfCallee * instructionInvocations);
  }

  /*
   * Set the total counter.
   */
  this->setFunctionTotalInstructions(&F, t);

  return;
}

} // namespace arcana::noelle
