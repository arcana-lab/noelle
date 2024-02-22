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
#ifndef NOELLE_SRC_CORE_DATAFLOW_DATAFLOWENGINE_H_
#define NOELLE_SRC_CORE_DATAFLOW_DATAFLOWENGINE_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/DataFlowResult.hpp"

namespace arcana::noelle {

class DataFlowEngine {
public:
  /*
   * Methods
   */
  DataFlowEngine();

  DataFlowResult *applyForward(
      Function *f,
      std::function<void(Instruction *, DataFlowResult *)> computeGEN,
      std::function<void(Instruction *, DataFlowResult *)> computeKILL,
      std::function<void(Instruction *inst, std::set<Value *> &IN)>
          initializeIN,
      std::function<void(Instruction *inst, std::set<Value *> &OUT)>
          initializeOUT,
      std::function<void(Instruction *inst,
                         Instruction *predecessor,
                         std::set<Value *> &IN,
                         DataFlowResult *df)> computeIN,
      std::function<void(Instruction *inst,
                         std::set<Value *> &OUT,
                         DataFlowResult *df)> computeOUT);

  DataFlowResult *applyForward(
      Function *f,
      std::function<void(Instruction *, DataFlowResult *)> computeGEN,
      std::function<void(Instruction *inst, std::set<Value *> &IN)>
          initializeIN,
      std::function<void(Instruction *inst, std::set<Value *> &OUT)>
          initializeOUT,
      std::function<void(Instruction *inst,
                         Instruction *predecessor,
                         std::set<Value *> &IN,
                         DataFlowResult *df)> computeIN,
      std::function<void(Instruction *inst,
                         std::set<Value *> &OUT,
                         DataFlowResult *df)> computeOUT);

  DataFlowResult *applyBackward(
      Function *f,
      std::function<void(Instruction *, DataFlowResult *)> computeGEN,
      std::function<void(Instruction *, DataFlowResult *)> computeKILL,
      std::function<void(Instruction *inst,
                         std::set<Value *> &IN,
                         DataFlowResult *df)> computeIN,
      std::function<void(Instruction *inst,
                         Instruction *successor,
                         std::set<Value *> &OUT,
                         DataFlowResult *df)> computeOUT);

  DataFlowResult *applyBackward(
      Function *f,
      std::function<void(Instruction *, DataFlowResult *)> computeGEN,
      std::function<void(Instruction *inst,
                         std::set<Value *> &IN,
                         DataFlowResult *df)> computeIN,
      std::function<void(Instruction *inst,
                         Instruction *successor,
                         std::set<Value *> &OUT,
                         DataFlowResult *df)> computeOUT);

protected:
  void computeGENAndKILL(
      Function *f,
      std::function<void(Instruction *, DataFlowResult *)> computeGEN,
      std::function<void(Instruction *, DataFlowResult *)> computeKILL,
      DataFlowResult *df);

private:
  DataFlowResult *applyCustomizableForwardAnalysis(
      Function *f,
      std::function<void(Instruction *, DataFlowResult *)> computeGEN,
      std::function<void(Instruction *, DataFlowResult *)> computeKILL,
      std::function<void(Instruction *inst, std::set<Value *> &IN)>
          initializeIN,
      std::function<void(Instruction *inst, std::set<Value *> &OUT)>
          initializeOUT,
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
      std::function<Instruction *(BasicBlock *bb)> getLastInstruction);

  DataFlowResult *applyGeneralizedAnalysis(
    Function *f,
    std::function<void(Instruction *, DataFlowResult *)> computeGEN,
    std::function<void(Instruction *, DataFlowResult *)> computeKILL,
    std::function<void(Instruction *inst, std::set<Value *> &IN)> initializeIN,
    std::function<void(Instruction *inst, std::set<Value *> &OUT)>
        initializeOUT,
    std::function< std::list<BasicBlock *> (BasicBlock *bb)> getPredecessors,
    std::function< std::list<BasicBlock *> (BasicBlock *bb)> getSuccessors,
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
    std::function<std::set<Value *> & (DataFlowResult * df, Instruction * instruction)> getInSetOfInst, 
    std::function<std::set<Value *> & (DataFlowResult * df, Instruction * instruction)> getOutSetOfInst,
    std::function<BasicBlock::iterator(BasicBlock*)> getEndIterator,
    std::function<void(BasicBlock::iterator&)>  incrementIterator );
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_DATAFLOW_DATAFLOWENGINE_H_
