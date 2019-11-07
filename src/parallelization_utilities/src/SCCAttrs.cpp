/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SCCAttrs.hpp"

using namespace llvm;

SCCAttrs::SCCAttrs (SCC *s)
  : scc{s}, isClonable{0}, hasIV{0},
    PHINodes{}, accumulators{}, controlFlowInsts{}, controlPairs{},
    singlePHI{nullptr}, singleAccumulator{nullptr}
  {

  /*
   * Collect the basic blocks of the instructions contained within SCC.
   */
  for (auto nodePair : this->scc->internalNodePairs()) {
    auto valueIncludedInSCC = nodePair.first;
    auto instIncludedInSCC = cast<Instruction>(valueIncludedInSCC);
    this->bbs.insert(instIncludedInSCC->getParent());
  }

  // Collect values actually contained in the strongly connected components,
  // ignoring ancillary values merged into the SCC object
  // collectSCCValues();

  /*
   * Collect the control flows of the SCC.
   */
  this->collectControlFlowInstructions();

  return;
}

void SCCAttrs::collectControlFlowInstructions (void){

  /*
   * Collect the terminators of the SCC that are involved in dependences.
   */
  for (auto iNodePair : this->scc->internalNodePairs()) {
    if (iNodePair.second->numOutgoingEdges() == 0) {
      continue;
    }
    auto currentValue = iNodePair.first;
    if (auto currentInst = dyn_cast<Instruction>(currentValue)){
      if (currentInst->isTerminator()){
        this->controlFlowInsts.insert(currentInst);
      }
    }
  }

  /*
   * Collect the (condition, jump) pairs.
   */
  for (auto term : this->controlFlowInsts) {
    assert(term->isTerminator());
    if (auto br = dyn_cast<BranchInst>(term)) {
      assert(br->isConditional()
        && "BranchInst with outgoing edges in an SCC must be conditional!");
      this->controlPairs.insert(std::make_pair(br->getCondition(), br));
    }
    if (auto switchI = dyn_cast<SwitchInst>(term)) {
      this->controlPairs.insert(std::make_pair(switchI->getCondition(), switchI));
    }
  }

  return ;
}

SCC * SCCAttrs::getSCC (void){
  return this->scc;
}

const std::pair<Value *, Instruction *> * SCCAttrs::getSingleInstructionThatControlLoopExit (void){
  if (this->controlPairs.size() != 1){
    return nullptr;
  }

  auto controlPair = &*this->controlPairs.begin();

  return controlPair;
}

// FIXME: Do not use
void SCCAttrs::collectSCCValues () {
  struct PathValue {
    PathValue *prev;
    Value *value;
    PathValue (Value *V, PathValue *PV = nullptr) : value{V}, prev{PV} {};
  };

  /*
   * Bookkeeping for later deletion and to avoid duplicate work
   */
  std::set<PathValue *> pathValues;
  std::set<Value *> valuesSeen;

  std::deque<PathValue *> toTraverse;
  auto topLevelNodes = scc->getTopLevelNodes(true);
  for (auto node : topLevelNodes) {
    node->getT()->print(errs() << "TOP LEVEL V: "); errs() << "\n";
    auto pathV = new PathValue(node->getT());
    pathValues.insert(pathV);
    toTraverse.push_front(pathV);
  }

  scc->print(errs(), "COLLECT: ", 0) << "\n";
  while (!toTraverse.empty()) {
    auto pathV = toTraverse.front();
    toTraverse.pop_front();
    // pathV->value->print(errs() << "Traversing V: "); errs() << "\n";

    bool isCycle = false;
    auto prevV = pathV->prev;
    while (prevV) {
      // prevV->value->print(errs() << "\t Prev V: "); errs() << "\n";
      if (pathV->value == prevV->value) {
        isCycle = true;
        break;
      }
      prevV = prevV->prev;
    }

    if (isCycle) {
      auto cycleV = pathV;
      while (cycleV != prevV) {
        stronglyConnectedDataValues.insert(cycleV->value);
        cycleV = cycleV->prev;
      }
      continue;
    }

    auto node = scc->fetchNode(pathV->value);
    for (auto edge : node->getOutgoingEdges()) {

      // Only trace paths across data dependencies, starting
      //  anew on newly encountered data values across control dependencies
      auto nextV = edge->getIncomingT();
      PathValue *nextPathV = nullptr;
      if (edge->isControlDependence()) {
        // nextV->print(errs() << "Control dependence traveling to: "); errs() << "\n";
        nextPathV = new PathValue(nextV);
      } else {
        // nextV->print(errs() << "Data dependence traveling to: "); errs() << "\n";
        nextPathV = new PathValue(nextV, pathV);
      }

      if (nextPathV) {
        pathValues.insert(nextPathV);
        toTraverse.push_front(nextPathV);
      }
    }
  }

  for (auto pathV : pathValues) delete pathV;

  for (auto dataV : stronglyConnectedDataValues) {
    dataV->print(errs() << "COLLECT: V: "); errs() << "\n";
  }
}
