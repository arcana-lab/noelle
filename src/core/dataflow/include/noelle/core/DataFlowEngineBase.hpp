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
#ifndef NOELLE_SRC_CORE_DATAFLOW_DATAFLOWENGINEBASE_H_
#define NOELLE_SRC_CORE_DATAFLOW_DATAFLOWENGINEBASE_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/DataFlowResult.hpp"

namespace arcana::noelle {

template <class T>
class DataFlowEngineBase {
public:
  /*
   * Methods
   */
  DataFlowEngineBase() = default;

  void computeGENAndKILL(
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

  DataFlowResult *applyGeneralizedForwardBase(
      Function *f,
      std::function<void(Instruction *, DataFlowResult *)> computeGEN,
      std::function<void(Instruction *, DataFlowResult *)> computeKILL,
      std::function<void(Instruction *inst, std::set<Value *> &IN)>
          initializeIN,
      std::function<void(Instruction *inst, std::set<Value *> &OUT)>
          initializeOUT,
      std::function<std::set<T>(T t)> getPredecessors,
      std::function<std::set<T>(T t)> getSuccessors,
      std::function<void(Instruction *inst,
                         Instruction *predecessor,
                         std::set<Value *> &IN,
                         DataFlowResult *df)> computeIN,
      std::function<void(Instruction *inst,
                         std::set<Value *> &OUT,
                         DataFlowResult *df)> computeOUT,

      ////
      std::function<std::list<T>(Function *f)> getWorkingList,
      std::function<Instruction *(T t)> getFirstInstruction,
      std::function<Instruction *(T t)> getLastInstruction,
      std::function<std::set<Value *> &(DataFlowResult *df,
                                        Instruction *instruction)>
          getInSetOfInst,
      std::function<std::set<Value *> &(DataFlowResult *df,
                                        Instruction *instruction)>
          getOutSetOfInst,
      std::function<Instruction *(Instruction *)> getNextInstruction) {

    /*
     * Initialize IN and OUT sets.
     */
    auto dfr = new DataFlowResult{};
    for (auto &bb : *f) {
      for (auto &i : bb) {
        auto &INSet = dfr->IN(&i);
        auto &OUTSet = dfr->OUT(&i);
        initializeIN(&i, INSet);
        initializeOUT(&i, OUTSet);
      }
    }

    /*
     * Compute the GENs and KILLs
     */
    computeGENAndKILL(f, computeGEN, computeKILL, dfr);

    /*
     * create the working list
     */
    std::list<T> workingList = getWorkingList(f);

    /*
     * compute the working list untill empty
     */
    std::unordered_set<T> computedOnce;

    while (!workingList.empty()) {

      /*
       * Fetch a compute unit
       */
      auto nodeT = workingList.front();

      /*
       * Remove from the working list
       */
      workingList.pop_front();

      /*
       * Fetch the first instruction
       */
      auto inst = getFirstInstruction(nodeT);

      /*
       * Fetch the current IN and OUT
       */
      auto &inSetOfInst = getInSetOfInst(dfr, inst);
      auto &outSetOfInst = getOutSetOfInst(dfr, inst);

      /*
       * Compute IN based on the predecessor
       */
      for (auto predecessorT : getPredecessors(nodeT)) {
        /*
         * Fetch the current predecessor Instruction
         */
        auto predecessorInst = getLastInstruction(predecessorT);

        /*
         * Compute IN
         */
        computeIN(inst, predecessorInst, inSetOfInst, dfr);
      }

      /*
       * Compute OUT
       */
      auto oldSizeOut = outSetOfInst.size();
      computeOUT(inst, outSetOfInst, dfr);

      /*
       * check if nodeT is computed before or has any changes
       */
      if (computedOnce.find(nodeT) == computedOnce.end()
          || (outSetOfInst.size() != oldSizeOut)) {
        /*
         * First Compute or Recompute
         */
        computedOnce.insert(nodeT);

        /*
         * Update the IN and OUT in the node
         */
        auto currentI = inst;
        auto predI = inst;

        while (currentI != getLastInstruction(nodeT)) {

          currentI = getNextInstruction(currentI);

          /*
           * Compute IN
           */
          auto &inSetOfI = getInSetOfInst(dfr, currentI);
          computeIN(currentI, predI, inSetOfI, dfr);

          /*
           * Compute OUT
           */
          auto &outSetOfI = getOutSetOfInst(dfr, currentI);
          computeOUT(currentI, outSetOfI, dfr);

          /*
           * update the predI
           */
          predI = currentI;
        }

        /*
         * add the successors to the workingList
         */
        for (auto succT : getSuccessors(nodeT)) {
          workingList.push_back(succT);
        }
      }
    }

    return dfr;
  }
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_DATAFLOW_DATAFLOWENGINEBASE_H_
