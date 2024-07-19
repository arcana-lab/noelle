/*
 * Copyright 2016 - 2023  Angelo Matni, Yian Su, Simone Campanoni
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
#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/TalkDown.hpp"
#include "arcana/noelle/core/PDGPrinter.hpp"
#include "arcana/noelle/core/PDGGenerator.hpp"

namespace arcana::noelle {

void PDGGenerator::embedPDGAsMetadata(PDG *pdg) {
  errs() << "Embed PDG as metadata\n";

  auto &C = this->M.getContext();
  std::unordered_map<Value *, MDNode *> nodeIDMap;

  embedNodesAsMetadata(pdg, C, nodeIDMap);
  embedEdgesAsMetadata(pdg, C, nodeIDMap);

  auto n = this->M.getOrInsertNamedMetadata("noelle.module.pdg");
  n->addOperand(MDNode::get(C, MDString::get(C, "true")));

  return;
}

void PDGGenerator::embedNodesAsMetadata(
    PDG *pdg,
    LLVMContext &C,
    std::unordered_map<Value *, MDNode *> &nodeIDMap) {
  uint64_t i = 0;
  std::unordered_map<Function *, std::unordered_map<uint64_t, Metadata *>>
      functionArgsIDMap;

  /*
   * Construct node to id map and embed metadata of instruction nodes to
   * instruction
   */
  for (auto v : pdg->getSortedValues()) {

    /*
     * Compute its ID.
     */
    auto id = ConstantInt::get(Type::getInt64Ty(C), i++);

    /*
     * Wrap its ID into a metadata node.
     */
    auto m = MDNode::get(C, ConstantAsMetadata::get(id));

    /*
     * Check if the current PDG node is an argument.
     */
    if (auto arg = dyn_cast<Argument>(v)) {

      /*
       * Register the current value as an argument in the metadata.
       */
      functionArgsIDMap[arg->getParent()][arg->getArgNo()] = m;

    } else if (auto inst = dyn_cast<Instruction>(v)) {

      /*
       * Attach the ID to the instruction.
       */
      inst->setMetadata("noelle.pdg.inst.id", m);
    }
    nodeIDMap[v] = m;
  }

  /*
   * Embed metadta of argument nodes to function
   */
  for (auto &funArgs : functionArgsIDMap) {
    std::vector<Metadata *> argsVec;
    for (uint64_t i = 0; i < funArgs.second.size(); i++) {
      argsVec.push_back(funArgs.second[i]);
    }

    auto m = MDTuple::get(C, argsVec);
    funArgs.first->setMetadata("noelle.pdg.args.id", m);
  }

  return;
}

void PDGGenerator::embedEdgesAsMetadata(
    PDG *pdg,
    LLVMContext &C,
    std::unordered_map<Value *, MDNode *> &nodeIDMap) {
  std::unordered_map<Function *, std::vector<Metadata *>> functionEdgesMap;

  /*
   * Construct edge metadata
   */
  for (auto &edge : pdg->getSortedDependences()) {

    /*
     * Fetch the next memory dependence.
     */
    if (!isa<MemoryDependence<Value, Value>>(edge)) {
      continue;
    }

    /*
     * Embed the current memory dependence into the IR.
     */
    auto edgeM = this->getEdgeMetadata(edge, C, nodeIDMap);
    if (auto arg = dyn_cast<Argument>(edge->getSrc())) {
      functionEdgesMap[arg->getParent()].push_back(edgeM);
    } else if (auto inst = dyn_cast<Instruction>(edge->getSrc())) {
      functionEdgesMap[inst->getFunction()].push_back(edgeM);
    } else {
      abort();
    }
  }

  /*
   * Embed metadata of edges to function
   */
  for (auto &funEdge : functionEdgesMap) {
    auto m = MDTuple::get(C, funEdge.second);
    funEdge.first->setMetadata("noelle.pdg.edges", m);
  }

  return;
}

} // namespace arcana::noelle
