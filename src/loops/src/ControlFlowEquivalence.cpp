/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "ControlFlowEquivalence.hpp"

using namespace llvm;

ControlFlowEquivalence::ControlFlowEquivalence (
  DominatorSummary &ds,
  LoopInfoSummary &lis,
  Function &F
) : DS{ds}, LIS{lis} {
  startBBs.insert(&F.getEntryBlock());
  calculateControlFlowEquivalences();
}

ControlFlowEquivalence::ControlFlowEquivalence (
  DominatorSummary &ds,
  LoopInfoSummary &lis,
  LoopSummary &ls
) : DS{ds}, LIS{lis} {
  startBBs.insert(ls.header);
  endBBs.insert(ls.exitBBs.begin(), ls.exitBBs.end());
  calculateControlFlowEquivalences();
}

/*
 * Goal: Traverse DT, merging equivalent sets where
 * node n's parent in the PDT is one of n's children in the DT
 */
void
ControlFlowEquivalence::calculateControlFlowEquivalences () {

  /*
   * Create trivial equivalence sets
   */
  std::queue<BasicBlock *> bbWorklist;
  std::set<BasicBlock *> bbSeen;
  for (auto start : startBBs) bbWorklist.push(start);
  while (!bbWorklist.empty()) {
    auto B = bbWorklist.front();
    bbWorklist.pop();

    if (bbSeen.find(B) != bbSeen.end()) continue;
    if (endBBs.find(B) != endBBs.end()) continue;
    bbSeen.insert(B);
    createEquivalenceSet(B);

    for (auto succB : successors(B)) {
      bbWorklist.push(succB);
    }
  }

  /*
   * Merge equivalence sets where:
   *   A |- B, B -| A, A & B are in the same inner-most loop
   */
  std::queue<DomNodeSummary *> dtWorklist;
  for (auto start : startBBs) dtWorklist.push(DS.DT.getNode(start));
  while (!dtWorklist.empty()) {
    auto dtNode = dtWorklist.front();
    dtWorklist.pop();

    /*
     * Consider this block A
     */
    auto dtBlock = dtNode->getBlock();
    // dtBlock->printAsOperand(errs() << "CFE: Iterating: "); errs() << "\n";
    if (endBBs.find(dtBlock) != endBBs.end()) continue;

    std::set<BasicBlock *> dtChildrenBlocks{};
    for (auto dtChildNode : dtNode->getChildren()) {
      dtChildrenBlocks.insert(dtChildNode->getBlock());
    }

    auto dtBlockLoop = LIS.bbToLoop.find(dtBlock);
    auto dtBlockInLoop = dtBlockLoop != LIS.bbToLoop.end();

    /*
     * Iterate PDT parents of A, those that qualify for condition B -| A
     */
    auto pdtNode = DS.PDT.getNode(dtBlock);
    auto pdtParentNode = pdtNode->getParent();
    while (pdtParentNode != nullptr) {
      auto pdtBlock = pdtParentNode->getBlock();
      pdtParentNode = pdtParentNode->getParent();

      /*
       * Check if condition A |- B is not held
       */
      if (dtChildrenBlocks.find(pdtBlock) == dtChildrenBlocks.end()) {
        break;
      } else {
        dtChildrenBlocks.clear();
        for (auto dtChildNode : DS.DT.getNode(pdtBlock)->getChildren()) {
          dtChildrenBlocks.insert(dtChildNode->getBlock());
        }
      }

      /*
       * Check if A and B are in the same inner-most loop
       */
      auto pdtBlockLoop = LIS.bbToLoop.find(pdtBlock);
      auto pdtBlockInLoop = pdtBlockLoop != LIS.bbToLoop.end();
      if (dtBlockInLoop ^ pdtBlockInLoop) continue;
      if (dtBlockInLoop && pdtBlockInLoop
          && dtBlockLoop->second != pdtBlockLoop->second) continue;

      mergeEquivalenceSets(dtBlock, pdtBlock);
      break;
    }

    /*
     * Queue all children to continue traversal
     */
    for (auto dtChildNode : dtNode->getChildren()) {
      // dtChildNode->getBlock()->printAsOperand(errs() << "CFE: Going to: "); errs() << "\n";
      dtWorklist.push(dtChildNode);
    }
  }
}

std::set<BasicBlock *> &
ControlFlowEquivalence::getEquivalences (BasicBlock *bb) {
  // TODO: Check if BB is in an equivalence set
  return *bbToEquivalence[bb];
}

raw_ostream &
ControlFlowEquivalence::print (
  raw_ostream &stream,
  std::string prefixToUse
) {
  stream << prefixToUse << "Control Flow Equivalent sets\n";
  for (auto &bbSet : equivalentBBs) {
    stream << prefixToUse << "Set:\n" << prefixToUse;
    for (auto bb : *bbSet.get()) {
      bb->printAsOperand(stream << " ", false);
    }
    stream << "\n";
  }
  return stream;
}

void
ControlFlowEquivalence::createEquivalenceSet (
  BasicBlock *singleB
) {
  auto eqSet = std::make_unique<std::set<BasicBlock *>>();
  eqSet->insert(singleB);
  bbToEquivalence[singleB] = equivalentBBs.insert(std::move(eqSet)).first->get();
}

void
ControlFlowEquivalence::mergeEquivalenceSets (
  BasicBlock *x,
  BasicBlock *y
) {
  // x->printAsOperand(errs() << " CFE: Merging: ", false);
  // y->printAsOperand(errs() << " with "); errs() << "\n";
  auto xSet = bbToEquivalence[x];
  bbToEquivalence[y]->insert(xSet->begin(), xSet->end());
  for (auto v : *xSet) {
    bbToEquivalence[v] = bbToEquivalence[y];
  }
}
