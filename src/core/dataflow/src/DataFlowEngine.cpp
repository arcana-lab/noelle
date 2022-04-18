/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/DataFlowEngine.hpp"

namespace llvm::noelle {

DataFlowEngine::DataFlowEngine (){
  return ;
}

DataFlowResult * DataFlowEngine::applyForward (
    Function *f,
    std::function<void (Instruction *, DataFlowResult *)> computeGEN,
    std::function<void (Instruction *inst, std::set<Value *>& IN)> initializeIN,
    std::function<void (Instruction *inst, std::set<Value *>& OUT)> initializeOUT,
    std::function<void (Instruction *inst, std::set<Value *>& IN, Instruction *predecessor, DataFlowResult *df)> computeIN,
    std::function<void (Instruction *inst, std::set<Value *>& OUT, DataFlowResult *df)> computeOUT
    ){

  /*
   * Define an empty KILL set.
   */
  auto computeKILL = [](Instruction *, DataFlowResult *) {
    return ;
  };

  /*
   * Run the data-flow analysis.
   */
  auto dfr = this->applyForward(f, computeGEN, computeKILL, initializeIN, initializeOUT, computeIN, computeOUT);

  return dfr;
}

DataFlowResult * DataFlowEngine::applyForward (
    Function *f,
    std::function<void (Instruction *, DataFlowResult *)> computeGEN,
    std::function<void (Instruction *, DataFlowResult *)> computeKILL,
    std::function<void (Instruction *inst, std::set<Value *>& IN)> initializeIN,
    std::function<void (Instruction *inst, std::set<Value *>& OUT)> initializeOUT,
    std::function<void (Instruction *inst, std::set<Value *>& IN, Instruction *predecessor, DataFlowResult *df)> computeIN,
    std::function<void (Instruction *inst, std::set<Value *>& OUT, DataFlowResult *df)> computeOUT
    ){

  /*
   * Define the customization.
   */
  auto appendBB = [](std::list<BasicBlock *> &workingList, BasicBlock *bb){ 
    workingList.push_back(bb);
  };

  auto getFirstInst = [](BasicBlock *bb) -> Instruction *{
    return &*bb->begin();
  };
  auto getLastInst = [](BasicBlock *bb) -> Instruction *{
    return bb->getTerminator();
  };

  /*
   * Run the pass.
   */
  auto dfaResult = this->applyCustomizableForwardAnalysis(
      f, 
      computeGEN, 
      computeKILL, 
      initializeIN, 
      initializeOUT, 
      computeIN, 
      computeOUT,
      appendBB,
      getFirstInst,
      getLastInst
      );

  return dfaResult;
}

DataFlowResult * DataFlowEngine::applyBackward (
    Function *f,
    std::function<void (Instruction *, DataFlowResult *)> computeGEN,
    std::function<void (std::set<Value *>& IN, Instruction *inst, DataFlowResult *df)> computeIN,
    std::function<void (std::set<Value *>& OUT, Instruction *successor, DataFlowResult *df)> computeOUT
    ){

  /*
   * Define an empty KILL set.
   */
  auto computeKILL = [](Instruction *, DataFlowResult *) {
    return ;
  };

  /*
   * Run the data-flow analysis.
   */
  auto dfr = this->applyBackward(f, computeGEN, computeKILL, computeIN, computeOUT);

  return dfr;
}

DataFlowResult * DataFlowEngine::applyBackward (
    Function *f,
    std::function<void (Instruction *, DataFlowResult *)> computeGEN,
    std::function<void (Instruction *, DataFlowResult *)> computeKILL,
    std::function<void (std::set<Value *>& IN, Instruction *inst, DataFlowResult *df)> computeIN,
    std::function<void (std::set<Value *>& OUT, Instruction *successor, DataFlowResult *df)> computeOUT
    ){

  /*
   * Compute the GENs and KILLs
   */
  auto df = new DataFlowResult{};
  computeGENAndKILL(f, computeGEN, computeKILL, df);

  /*
   * Compute the IN and OUT
   *
   * Create the working list by adding all basic blocks to it.
   */
  std::unordered_set<BasicBlock *> computedOnce;
  std::list<BasicBlock *> workingList;
  for (auto& bb : *f){
    workingList.push_front(&bb);
  }

  /* 
   * Compute the INs and OUTs iteratively until the working list is empty.
   */
  while (!workingList.empty()){

    /* 
     * Fetch a basic block that needs to be processed.
     */
    auto bb = workingList.front();
    workingList.pop_front();

    /* 
     * Fetch the last instruction of the current basic block.
     */
    auto inst = bb->getTerminator();
    assert(inst != nullptr);

    /* 
     * Fetch IN[inst], OUT[inst], GEN[inst], and KILL[inst]
     */
    auto& inSetOfInst = df->IN(inst);
    auto& outSetOfInst = df->OUT(inst);
    auto& genSetOfInst = df->GEN(inst);
    auto& killSetOfInst = df->KILL(inst);

    /* 
     * Compute OUT[inst]
     */
    for (auto successorBB : successors(bb)){

      /* 
       * Fetch the current successor of "inst".
       */
      auto successorInst = &*successorBB->begin();

      /* 
       * Compute OUT[inst]
       */
      computeOUT(outSetOfInst, successorInst, df);
    }

    /* 
     * Compute IN[inst]
     */
    auto oldSize = inSetOfInst.size();
    computeIN(inSetOfInst, inst, df);

    /* 
     * Check if IN[inst] changed.
     */
    if (  false
        || (inSetOfInst.size() > oldSize)
        || (computedOnce.find(bb) == computedOnce.end())
       ){

      /*
       * Remember that we have now computed this basic block.
       */
      computedOnce.insert(bb);

      /* 
       * Propagate the new IN[inst] to the rest of the instructions of the current basic block.
       */
      BasicBlock::iterator iter(inst);
      auto succI = cast<Instruction>(inst);
      while (iter != bb->begin()){

        /*
         * Move the iterator.
         */
        iter--;

        /*
         * Fetch the current instruction.
         */
        auto i = &*iter;

        /* 
         * Compute OUT[i]
         */
        auto& outSetOfI = df->OUT(i);
        computeOUT(outSetOfI, succI, df);

        /* 
         * Compute IN[i] 
         */
        auto& inSetOfI = df->IN(i);
        computeIN(inSetOfI, i, df);

        /*
         * Update the successor.
         */
        succI = i;
      }

      /* 
       * Add predecessors of the current basic block to the working list.
       */
      for (auto predBB : predecessors(bb)){
        workingList.push_back(predBB);
      }
    }
  }

  return df;
}

void DataFlowEngine::computeGENAndKILL (
    Function *f, 
    std::function<void (Instruction *, DataFlowResult *)> computeGEN,
    std::function<void (Instruction *, DataFlowResult *)> computeKILL,
    DataFlowResult *df
    ){

  /*
   * Compute the GENs and KILLs
   */
  for (auto& bb : *f){
    for (auto& i : bb){
      computeGEN(&i, df);
      computeKILL(&i, df);
    }
  }

  return ;
}

DataFlowResult * DataFlowEngine::applyCustomizableForwardAnalysis (
    Function *f,
    std::function<void (Instruction *, DataFlowResult *)> computeGEN,
    std::function<void (Instruction *, DataFlowResult *)> computeKILL,
    std::function<void (Instruction *inst, std::set<Value *>& IN)> initializeIN,
    std::function<void (Instruction *inst, std::set<Value *>& OUT)> initializeOUT,
    std::function<void (Instruction *inst, std::set<Value *>& IN, Instruction *predecessor, DataFlowResult *df)> computeIN,
    std::function<void (Instruction *inst, std::set<Value *>& OUT, DataFlowResult *df)> computeOUT,
    std::function<void (std::list<BasicBlock *> &workingList, BasicBlock *bb)> appendBB,
    std::function<Instruction * (BasicBlock *bb)> getFirstInstruction,
    std::function<Instruction * (BasicBlock *bb)> getLastInstruction
    ){

  /*
   * Initialize IN and OUT sets.
   */
  auto df = new DataFlowResult{};
  for (auto& bb : *f){
    for (auto& i : bb){
      auto& INSet = df->IN(&i);
      auto& OUTSet = df->OUT(&i);
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
  std::unordered_map<BasicBlock *, bool> worklingListContent;
  for (auto& bb : *f){
    appendBB(workingList, &bb);
    worklingListContent[&bb] = true;
  }

  /* 
   * Compute the INs and OUTs iteratively until the working list is empty.
   */
  std::unordered_map<Instruction *, bool> alreadyVisited;
  while (!workingList.empty()){

    /* 
     * Fetch a basic block that needs to be processed.
     */
    auto bb = workingList.front();
    assert(worklingListContent[bb] == true);

    /*
     * Remove the basic block from the workingList.
     */
    workingList.pop_front();
    worklingListContent[bb] = false;

    /* 
     * Fetch the first instruction of the basic block.
     */
    auto inst = getFirstInstruction(bb);

    /* 
     * Fetch IN[inst], OUT[inst], GEN[inst], and KILL[inst]
     */
    auto& inSetOfInst = df->IN(inst);
    auto& outSetOfInst = df->OUT(inst);
    auto& genSetOfInst = df->GEN(inst);
    auto& killSetOfInst = df->KILL(inst);

    /* 
     * Compute the IN of the first instruction of the current basic block.
     */
    for (auto predecessorBB : predecessors(bb)){

      /* 
       * Fetch the current predecessor of "inst".
       */
      auto predecessorInst = getLastInstruction(predecessorBB);

      /* 
       * Compute IN[inst]
       */
      computeIN(inst, inSetOfInst, predecessorInst, df);
    }

    /* 
     * Compute OUT[inst]
     */
    auto oldSize = outSetOfInst.size();
    computeOUT(inst, outSetOfInst, df);

    /* Check if the OUT of the first instruction of the current basic block changed.
    */
    if (  false
        || (!alreadyVisited[inst])
        || (outSetOfInst.size() != oldSize)
       ){
      alreadyVisited[inst] = true;

      /* 
       * Propagate the new OUT[inst] to the rest of the instructions of the current basic block.
       */
      BasicBlock::iterator iter(inst);
      auto predI = inst;
      iter++;
      while (iter != bb->end()){

        /*
         * Fetch the current instruction.
         */
        auto i = &*iter;

        /* 
         * Compute IN[i]
         */
        auto& inSetOfI = df->IN(i);
        computeIN(i, inSetOfI, predI, df);

        /* 
         * Compute OUT[i] 
         */
        auto& outSetOfI = df->OUT(i);
        computeOUT(i, outSetOfI, df);

        /*
         * Update the predecessor.
         */
        predI = i;

        /*
         * Move the iterator.
         */
        iter++;
      } 

      /* 
       * Add successors of the current basic block to the working list.
       */
      for (auto succBB : successors(bb)){
        if (worklingListContent[succBB] == true){
          continue ;
        }
        workingList.push_back(succBB);
        worklingListContent[succBB] = true;
      }
    }
  }

  return df;
}

}
