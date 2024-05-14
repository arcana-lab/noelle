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
#include "noelle/core/DataFlowEngineBase.hpp"
#include "noelle/core/DataFlowEngineCFG.hpp"

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
  auto getFirstInst = [](BasicBlock *bb) -> Instruction * {
    return &*bb->begin();
  };

  auto getLastInst = [](BasicBlock *bb) -> Instruction * {
    return bb->getTerminator();
  };

  auto getPredecessors = [](BasicBlock *bb) -> std::set<BasicBlock *> {
    std::set<BasicBlock *> Predecessors;
    for (auto predecessor : predecessors(bb)) {
      Predecessors.insert(predecessor);
    }
    return Predecessors;
  };

  auto getSuccessors = [](BasicBlock *bb) -> std::set<BasicBlock *> {
    std::set<BasicBlock *> Successors;
    for (auto predecessor : successors(bb)) {
      Successors.insert(predecessor);
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

  auto getWorkingList = [](Function *f) {
    std::list<BasicBlock *> workingList;
    for (auto &bb : *f) {
      workingList.push_back(&bb);
    }
    return workingList;
  };

  auto getNextInstruction = [](Instruction *inst) {
    BasicBlock::iterator iter(inst);
    iter++;
    return &*(iter);
  };

  /*
   * Run the pass.
   */
  auto dfe = DataFlowEngineBase<BasicBlock *>();

  auto dfr = dfe.applyGeneralizedForwardBase(f,
                                             computeGEN,
                                             computeKILL,
                                             initializeIN,
                                             initializeOUT,
                                             getPredecessors,
                                             getSuccessors,
                                             computeIN,
                                             computeOUT,
                                             getWorkingList,
                                             getFirstInst,
                                             getLastInst,
                                             inSetOfInst,
                                             outSetOfInst,
                                             getNextInstruction);

  return dfr;
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
  auto getPredecessors = [](BasicBlock *bb) -> std::set<BasicBlock *> {
    std::set<BasicBlock *> Successors;
    for (auto predecessor : successors(bb)) {
      Successors.insert(predecessor);
    }
    return Successors;
  };

  auto getSuccessors = [](BasicBlock *bb) -> std::set<BasicBlock *> {
    std::set<BasicBlock *> Predecessors;
    for (auto predecessor : predecessors(bb)) {
      Predecessors.insert(predecessor);
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

  auto getWorkingList = [](Function *f) {
    std::list<BasicBlock *> workingList;
    for (auto &bb : *f) {
      workingList.push_front(&bb);
    }
    return workingList;
  };

  auto getNextInstruction = [](Instruction *inst) {
    BasicBlock::iterator iter(inst);
    iter--;
    return &*(iter);
  };

  auto dfe = DataFlowEngineBase<BasicBlock *>();

  auto dfr = dfe.applyGeneralizedForwardBase(f,
                                             computeGEN,
                                             computeKILL,
                                             initializeIN,
                                             initializeOUT,
                                             getPredecessors,
                                             getSuccessors,
                                             computeOUT,
                                             computeIN,
                                             getWorkingList,
                                             getFirstInst,
                                             getLastInst,
                                             inSetOfInst,
                                             outSetOfInst,
                                             getNextInstruction);

  return dfr;
}

} // namespace arcana::noelle
