/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include <set>
#include <queue>

#include "llvm/Support/raw_ostream.h"

#include "../include/PDG.hpp"

llvm::PDG::PDG() {}

llvm::PDG::~PDG() {
  for (auto *edge : allEdges)
    if (edge) delete edge;
  for (auto *node : allNodes)
    if (node) delete node;
}

void llvm::PDG::populateNodesOf (Module &M) {

  /*
   * Create a node per instruction and function argument
   */
  for (auto &F : M) {
    addNodesOf(F);
  }

  /* 
   * Set the entry node: the first instruction of the function "main"
   */
  auto mainF = M.getFunction("main");
  assert(mainF != nullptr);
  setEntryPointAt(*mainF);
}

void llvm::PDG::populateNodesOf (Function &F) {
  addNodesOf(F);
  setEntryPointAt(F);
}

void llvm::PDG::addNodesOf (Function &F) {
  for (auto &arg : F.args()) addNode(cast<Value>(&arg), /*inclusion=*/ true);

  for (auto &B : F) {
    for (auto &I : B) {
      addNode(cast<Value>(&I), /*inclusion=*/ true);
    }
  }
}

void llvm::PDG::setEntryPointAt (Function &F) {
  auto entryInstr = &*(F.begin()->begin());
  entryNode = internalNodeMap[entryInstr];
  assert(entryNode != nullptr);
}

llvm::DGEdge<Value> * llvm::PDG::addEdge (Value *from, Value *to) { 
  return this->DG<Value>::addEdge(from, to); 
}

PDG *llvm::PDG::createFunctionSubgraph(Function &F) {
  if (F.empty()) return nullptr;
  auto functionPDG = new PDG();

  /*
   * Create a node per instruction and argument of the function
   */
  functionPDG->addNodesOf(F);

  /* 
   * Set the entry node: the first instruction of function F
   */
  functionPDG->setEntryPointAt(F);

  /*
   * Recreate all edges connected to internal nodes of function
   */
  copyEdgesInto(functionPDG, /*linkToExternal=*/ true);

  return functionPDG;
}

PDG *llvm::PDG::createLoopsSubgraph(Loop *loop) {
  auto loopsPDG = new PDG();

  /*
   * Create a node per instruction within loops of LI only
   */
  for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi) {
    for (auto &I : **bbi) {
      loopsPDG->addNode(cast<Value>(&I), /*inclusion=*/ true);
    }
  }

  /*
   * Set the entry node: the first instruction of one of the top level loops (See include/llvm/Analysis/LoopInfo.h:653)
   */
  BasicBlock *bbBegin = *(loop->block_begin());
  loopsPDG->entryNode = loopsPDG->internalNodeMap[&*(bbBegin->begin())];
  assert(loopsPDG->entryNode != nullptr);

  /*
   * Recreate all edges connected to internal nodes of loop
   */
  copyEdgesInto(loopsPDG, /*linkToExternal=*/ true);

  return loopsPDG;
}

PDG *llvm::PDG::createSubgraphFromValues(std::vector<Value *> &valueList, bool linkToExternal) {
  if (valueList.empty()) return nullptr;
  auto newPDG = new PDG();

  for (auto &V : valueList) {
    newPDG->addNode(V, /*inclusion=*/ true);
  }

  newPDG->entryNode = newPDG->internalNodeMap[*(valueList.begin())];
  assert(newPDG->entryNode != nullptr);

  copyEdgesInto(newPDG, linkToExternal);

  return newPDG;
}

void llvm::PDG::copyEdgesInto(PDG *newPDG, bool linkToExternal) {
  for (auto *oldEdge : allEdges)
  {
    auto nodePair = oldEdge->getNodePair();
    auto fromT = nodePair.first->getT();
    auto toT = nodePair.second->getT();

    /*
     * Check whether edge belongs to nodes within function F
     */
    bool fromInclusion = newPDG->isInternal(fromT);
    bool toInclusion = newPDG->isInternal(toT);
    if (!fromInclusion && !toInclusion) continue;
    if (!linkToExternal && (!fromInclusion || !toInclusion)) continue;
    
    /*
     * Create appropriate external nodes and associate edge to them
     */
    auto newFromNode = newPDG->fetchOrAddNode(fromT, fromInclusion);
    auto newToNode = newPDG->fetchOrAddNode(toT, toInclusion);

    /*
     * Copy edge to match properties (mem/var, must/may, RAW/WAW/WAR/control)
     */
    newPDG->copyAddEdge(*oldEdge);
   }
}
