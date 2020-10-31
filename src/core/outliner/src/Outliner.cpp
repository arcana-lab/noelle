/*
 * Copyright 2020 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "OutlinerPass.hpp"

namespace llvm::noelle {

  Outliner::Outliner (){
  
    return ;
  }

  Function * outline (std::unordered_set<Instruction *> const & instructionsToOutline, Instruction *injectCallJustBeforeThis){
    //TODO
    return nullptr;
  }

  Function * outline (std::unordered_set<BasicBlock *> const & basicBlocksToOutline, Instruction *injectCallJustBeforeThis){
    //TODO
    //1. Collect data about basicblocks
    // - check dependencies that enter basic blocks from the outside - input variables 
    // - define input variables which are not modified inside basic block (pass by value)
    // - define input variables which are modified inside basic block (pass by reference)
    //2. Create noinline function
    //3. Use llvm to rewire all variables
    //4. Return new function


    std::unordered_set<Instruction *> instructions; // we will need this for something?
    std::unordered_set<Instruction *> inputs;
    std::unordered_set<Instruction *> outputs;

    for (auto *bb : basicBlocksToOutline){
      for(auto &i : *bb) {
        instructions.insert(&i); 
        for(auto &op : i.operands()){
          Value * val = op.get();
          if(!isa<Instruction>(val)){
            continue;
          }

          Instruction *op_instr = dyn_cast<Instruction>(val);
          if(basicBlocksToOutline.find(op_instr->getParent()) == basicBlocksToOutline.end() ){
            //now figure out if it's modified ????
            //if it isn't
            ///inputs.insert(op_instr);


            //but if it is
            //outputs.insert(op_instr);

          }
        }
      }
    }


    return nullptr;
  }

}
