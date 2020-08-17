/*
 * Copyright 2016 - 2020  Angelo Matni, Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"

#include "Util/SVFModule.h"
#include "WPA/Andersen.h"
#include "TalkDown.hpp"
#include "PDGPrinter.hpp"
#include "PDGAnalysis.hpp"

using namespace llvm;

void PDGAnalysis::embedPDGAsMetadata(PDG *pdg) {
  errs() << "Embed PDG as Metadata\n";

  auto &C = this->M->getContext();
  unordered_map<Value *, MDNode *> nodeIDMap;

  embedNodesAsMetadata(pdg, C, nodeIDMap);
  embedEdgesAsMetadata(pdg, C, nodeIDMap);

  auto n = this->M->getOrInsertNamedMetadata("noelle.module.pdg");
  n->addOperand(MDNode::get(C, MDString::get(C, "true")));

  return;
}

void PDGAnalysis::embedNodesAsMetadata(PDG *pdg, LLVMContext &C, unordered_map<Value *, MDNode *> &nodeIDMap) {
  uint64_t i = 0;
  unordered_map<Function *, unordered_map<uint64_t, Metadata *>> functionArgsIDMap;

  /*
   * Construct node to id map and embed metadata of instruction nodes to instruction
   */
  for (auto &node : pdg->getNodes()) {
    Value *v = node->getT();
    Constant *id = ConstantInt::get(Type::getInt64Ty(C), i++);
    MDNode *m = MDNode::get(C, ConstantAsMetadata::get(id));
    if (Argument *arg = dyn_cast<Argument>(v)) {
      functionArgsIDMap[arg->getParent()][arg->getArgNo()] = m;
    }
    else if (Instruction *inst = dyn_cast<Instruction>(v)) {
      inst->setMetadata("noelle.pdg.inst.id", m);
    }
    nodeIDMap[v] = m;
  }

  /*
   * Embed metadta of argument nodes to function
   */
  for (auto &funArgs : functionArgsIDMap) {
    vector<Metadata *> argsVec;
    for (uint64_t i = 0; i < funArgs.second.size(); i++) {
      argsVec.push_back(funArgs.second[i]);
    }

    MDNode *m = MDTuple::get(C, argsVec);
    funArgs.first->setMetadata("noelle.pdg.args.id", m);
  }

  return;
}

void PDGAnalysis::embedEdgesAsMetadata(PDG *pdg, LLVMContext &C, unordered_map<Value *, MDNode *> &nodeIDMap) {
  unordered_map<Function *, vector<Metadata *>> functionEdgesMap;

  /*
   * Construct edge metadata
   */
  for (auto &edge : pdg->getEdges()) {
    MDNode *edgeM = getEdgeMetadata(edge, C, nodeIDMap);
    if (Argument *arg = dyn_cast<Argument>(edge->getOutgoingT())) {
      functionEdgesMap[arg->getParent()].push_back(edgeM);
    }
    else if (Instruction *inst = dyn_cast<Instruction>(edge->getOutgoingT())) {
      functionEdgesMap[inst->getFunction()].push_back(edgeM);
    }
  }

  /*
   * Embed metadata of edges to function
   */
  for (auto &funEdge : functionEdgesMap) {
    MDNode *m = MDTuple::get(C, funEdge.second);
    funEdge.first->setMetadata("noelle.pdg.edges", m);
  }

  return;
}
