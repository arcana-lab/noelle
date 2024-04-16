/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
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
#include "noelle/core/DataFlowEngine.hpp"

namespace arcana::noelle {

DataFlowEngine::DataFlowEngine() {
  return;
}

DataFlowResult *DataFlowEngine::applyForward(
    Function *f,
    std::function<void(Instruction *, DataFlowResult *)> computeGEN,
    std::function<void(Instruction *inst, std::set<Value *> &IN)> initializeIN,
    std::function<void(Instruction *inst, std::set<Value *> &OUT)>
        initializeOUT,
    std::function<void(Instruction *inst,
                       Instruction *predecessor,
                       std::set<Value *> &IN,
                       DataFlowResult *df)> computeIN,
    std::function<void(Instruction *inst,
                       std::set<Value *> &OUT,
                       DataFlowResult *df)> computeOUT) {

  /*
   * Define an empty KILL set.
   */
  auto computeKILL = [](Instruction *, DataFlowResult *) { return; };

  /*
   * Run the data-flow analysis.
   */
  auto dfr = this->applyForward(f,
                                computeGEN,
                                computeKILL,
                                initializeIN,
                                initializeOUT,
                                computeIN,
                                computeOUT);

  return dfr;
}

DataFlowResult *DataFlowEngine::applyForward(
    Function *f,
    std::function<void(Instruction *, DataFlowResult *)> computeGEN,
    std::function<void(Instruction *, DataFlowResult *)> computeKILL,
    std::function<void(Instruction *inst, std::set<Value *> &IN)> initializeIN,
    std::function<void(Instruction *inst, std::set<Value *> &OUT)>
        initializeOUT,
    std::function<void(Instruction *inst,
                       Instruction *predecessor,
                       std::set<Value *> &IN,
                       DataFlowResult *df)> computeIN,
    std::function<void(Instruction *inst,
                       std::set<Value *> &OUT,
                       DataFlowResult *df)> computeOUT) {

  /*
   * Define the customization.
   */
  auto appendBB = [](std::list<BasicBlock *> &workingList, BasicBlock *bb) {
    workingList.push_back(bb);
  };

  auto getFirstInst = [](BasicBlock *bb) -> Instruction * {
    return &*bb->begin();
  };

  auto getLastInst = [](BasicBlock *bb) -> Instruction * {
    return bb->getTerminator();
  };

  auto getPredecessors = [](BasicBlock *bb) -> std::list<BasicBlock *> {
    std::list<BasicBlock *> Predecessors;
    for (auto predecessor : predecessors(bb)) {
      Predecessors.push_back(predecessor);
    }
    return Predecessors;
  };

  auto getSuccessors = [](BasicBlock *bb) -> std::list<BasicBlock *> {
    std::list<BasicBlock *> Successors;
    for (auto predecessor : successors(bb)) {
      Successors.push_back(predecessor);
    }
    return Successors;
  };

  auto inSetOfInst = [](DataFlowResult *df,
                        Instruction *inst) -> std::set<Value *> & {
    return df->IN(inst);
  };

  auto outSetOfInst = [](DataFlowResult *df,
                         Instruction *inst) -> std::set<Value *> & {
    return df->OUT(inst);
  };

  auto getEndIterator = [](BasicBlock *bb) -> BasicBlock::iterator {
    return bb->end();
  };

  auto incrementIterator = [](BasicBlock::iterator &iter) { iter++; };

  /*
   * Run the pass.
   */

  auto dfa = this->applyGeneralizedForwardAnalysis(f,
                                                   computeGEN,
                                                   computeKILL,
                                                   initializeIN,
                                                   initializeOUT,
                                                   getPredecessors,
                                                   getSuccessors,
                                                   computeIN,
                                                   computeOUT,
                                                   appendBB,
                                                   getFirstInst,
                                                   getLastInst,
                                                   inSetOfInst,
                                                   outSetOfInst,
                                                   getEndIterator,
                                                   incrementIterator);
  return dfa;
}

DataFlowResult *DataFlowEngine::applyBackward(
    Function *f,
    std::function<void(Instruction *, DataFlowResult *)> computeGEN,
    std::function<void(Instruction *inst,
                       std::set<Value *> &IN,
                       DataFlowResult *df)> computeIN,
    std::function<void(Instruction *inst,
                       Instruction *successor,
                       std::set<Value *> &OUT,
                       DataFlowResult *df)> computeOUT) {

  /*
   * Define an empty KILL set.
   */
  auto computeKILL = [](Instruction *, DataFlowResult *) { return; };

  /*
   * Run the data-flow analysis.
   */
  auto dfr =
      this->applyBackward(f, computeGEN, computeKILL, computeIN, computeOUT);

  return dfr;
}

DataFlowResult *DataFlowEngine::applyBackward(
    Function *f,
    std::function<void(Instruction *, DataFlowResult *)> computeGEN,
    std::function<void(Instruction *, DataFlowResult *)> computeKILL,
    std::function<void(Instruction *inst,
                       std::set<Value *> &IN,
                       DataFlowResult *df)> computeIN,
    std::function<void(Instruction *inst,
                       Instruction *successor,
                       std::set<Value *> &OUT,
                       DataFlowResult *df)> computeOUT) {

  /*
   * Define the customization
   */
  auto appendBB = [](std::list<BasicBlock *> &workingList, BasicBlock *bb) {
    workingList.push_front(bb);
  };

  auto getPredecessors = [](BasicBlock *bb) -> std::list<BasicBlock *> {
    std::list<BasicBlock *> Successors;
    for (auto predecessor : successors(bb)) {
      Successors.push_back(predecessor);
    }
    return Successors;
  };

  auto getSuccessors = [](BasicBlock *bb) -> std::list<BasicBlock *> {
    std::list<BasicBlock *> Predecessors;
    for (auto predecessor : predecessors(bb)) {
      Predecessors.push_back(predecessor);
    }
    return Predecessors;
  };

  auto getFirstInst = [](BasicBlock *bb) -> Instruction * {
    return bb->getTerminator();
  };

  auto getLastInst = [](BasicBlock *bb) -> Instruction * {
    return &*bb->begin();
  };

  auto initializeIN = [](Instruction *inst, std::set<Value *> &IN) { return; };

  auto initializeOUT = [](Instruction *inst, std::set<Value *> &OUT) {
    return;
  };

  auto inSetOfInst = [](DataFlowResult *df,
                        Instruction *inst) -> std::set<Value *> & {
    return df->OUT(inst);
  };

  auto outSetOfInst = [](DataFlowResult *df,
                         Instruction *inst) -> std::set<Value *> & {
    return df->IN(inst);
  };

  auto getEndIterator = [](BasicBlock *bb) -> BasicBlock::iterator {
    return bb->begin();
  };

  auto incrementIterator = [](BasicBlock::iterator &iter) { iter--; };

  auto dfr = this->applyGeneralizedForwardAnalysis(f,
                                                   computeGEN,
                                                   computeKILL,
                                                   initializeIN,
                                                   initializeOUT,
                                                   getPredecessors,
                                                   getSuccessors,
                                                   computeOUT,
                                                   computeIN,
                                                   appendBB,
                                                   getFirstInst,
                                                   getLastInst,
                                                   inSetOfInst,
                                                   outSetOfInst,
                                                   getEndIterator,
                                                   incrementIterator);

  return dfr;
}

void DataFlowEngine::computeGENAndKILL(
    Function *f,
    std::function<void(Instruction *, DataFlowResult *)> computeGEN,
    std::function<void(Instruction *, DataFlowResult *)> computeKILL,
    DataFlowResult *df) {

  /*
   * Compute the GENs and KILLs
   */
  for (auto &bb : *f) {
    for (auto &i : bb) {
      computeGEN(&i, df);
      computeKILL(&i, df);
    }
  }

  return;
}

DataFlowResult *DataFlowEngine::applyGeneralizedForwardAnalysis(
    Function *f,
    std::function<void(Instruction *, DataFlowResult *)> computeGEN,
    std::function<void(Instruction *, DataFlowResult *)> computeKILL,
    std::function<void(Instruction *inst, std::set<Value *> &IN)> initializeIN,
    std::function<void(Instruction *inst, std::set<Value *> &OUT)>
        initializeOUT,
    std::function<std::list<BasicBlock *>(BasicBlock *bb)> getPredecessors,
    std::function<std::list<BasicBlock *>(BasicBlock *bb)> getSuccessors,
    std::function<void(Instruction *inst,
                       Instruction *predecessor,
                       std::set<Value *> &IN,
                       DataFlowResult *df)> computeIN,
    std::function<void(Instruction *inst,
                       std::set<Value *> &OUT,
                       DataFlowResult *df)> computeOUT,
    std::function<void(std::list<BasicBlock *> &workingList, BasicBlock *bb)>
        appendBB,
    std::function<Instruction *(BasicBlock *bb)> getFirstInstruction,
    std::function<Instruction *(BasicBlock *bb)> getLastInstruction,
    std::function<std::set<Value *> &(DataFlowResult *df,
                                      Instruction *instruction)> getInSetOfInst,
    std::function<std::set<Value *> &(DataFlowResult *df,
                                      Instruction *instruction)>
        getOutSetOfInst,
    std::function<BasicBlock::iterator(BasicBlock *)> getEndIterator,
    std::function<void(BasicBlock::iterator &)> incrementIterator) {

  /*
   * Initialize IN and OUT sets.
   */
  auto df = new DataFlowResult{};
  for (auto &bb : *f) {
    for (auto &i : bb) {
      auto &INSet = df->IN(&i);
      auto &OUTSet = df->OUT(&i);
      initializeIN(&i, INSet);
      initializeOUT(&i, OUTSet);
    }
  }

  /*
   * Compute the GENs and KILLs
   */
  computeGENAndKILL(f, computeGEN, computeKILL, df);

  /*
   * Compute the IN and OUT
   *
   * Create the working list by adding all basic blocks to it.
   */
  std::list<BasicBlock *> workingList;
  for (auto &bb : *f) {
    appendBB(workingList, &bb);
  }

  /*
   * Compute the INs and OUTs iteratively until the working list is empty.
   */
  std::unordered_set<BasicBlock *> computedOnce;

  while (!workingList.empty()) {

    /*
     * Fetch a basic block that needs to be processed.
     */
    auto bb = workingList.front();

    /*
     * Remove the basic block from the workingList.
     */
    workingList.pop_front();

    /*
     * Fetch the first instruction of the basic block.
     */
    auto inst = getFirstInstruction(bb);

    /*
     * Fetch IN[inst], OUT[inst], GEN[inst], and KILL[inst]
     */
    auto &inSetOfInst = getInSetOfInst(df, inst);
    auto &outSetOfInst = getOutSetOfInst(df, inst);

    /*
     * Compute the IN of the first instruction of the current basic block.
     */
    for (auto predecessorBB : getPredecessors(bb)) {

      /*
       * Fetch the current predecessor of "inst".
       */
      auto predecessorInst = getLastInstruction(predecessorBB);

      /*
       * Compute IN[inst]
       */
      computeIN(inst, predecessorInst, inSetOfInst, df);
    }

    /*
     * Compute OUT[inst]
     */
    auto oldSizeOut = outSetOfInst.size();
    computeOUT(inst, outSetOfInst, df);

    /* Check if the OUT of the first instruction of the current basic block
     * changed.
     */
    if ((computedOnce.find(bb) == computedOnce.end())
        || (outSetOfInst.size() != oldSizeOut)) {

      computedOnce.insert(bb);

      /*
       * Propagate the new OUT[inst] to the rest of the instructions of the
       * current basic block.
       */
      BasicBlock::iterator iter(inst);
      auto predI = cast<Instruction>(inst);
      while (iter != getEndIterator(bb)) {

        /*
         * Move the iterator.
         */
        incrementIterator(iter);

        /*
         * Fetch the current instruction.
         */
        auto i = &*iter;

        /*
         * Compute IN[i]
         */
        auto &inSetOfI = getInSetOfInst(df, i);
        computeIN(i, predI, inSetOfI, df);

        /*
         * Compute OUT[i]
         */
        auto &outSetOfI = getOutSetOfInst(df, i);
        computeOUT(i, outSetOfI, df);

        /*
         * Update the predecessor.
         */
        predI = i;
      }

      /*
       * Add successors of the current basic block to the working list.
       */
      for (auto succBB : getSuccessors(bb)) {
        workingList.push_back(succBB);
      }
    }
  }

  return df;
}

} // namespace arcana::noelle
