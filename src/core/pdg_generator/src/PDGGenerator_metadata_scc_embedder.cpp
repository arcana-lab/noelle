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
#include "arcana/noelle/core/SCCDAG.hpp"

namespace arcana::noelle {

void PDGGenerator::embedSCCAsMetadata(PDG *pdg) {
  errs() << "Embed SCCs as metadata\n";

  auto &C = this->M.getContext();

  auto n = this->M.getOrInsertNamedMetadata("noelle.module.pdg.scc");
  n->addOperand(MDNode::get(C, MDString::get(C, "true")));

  auto DAG = SCCDAG(pdg);
  std::unordered_map<uint32_t, MDNode *> indexToIndexMD;

  /*
   * Associate every instruction to the value of its SCC Index
   */
  for (auto v : pdg->getSortedValues()) {
    if (auto inst = dyn_cast<Instruction>(v)) {
      auto sccIndex = DAG.getSCCIndex(DAG.sccOfValue(inst));

      auto indexMD = indexToIndexMD.find(sccIndex);
      if (indexMD != indexToIndexMD.end()) {

        /*
         * The metadata node for this sccIndex is already available
         */
        inst->setMetadata("noelle.pdg.scc.id", indexMD->second);
      } else {

        /*
         * Wrap this SCC into a new metadata node.
         */
        auto id = ConstantInt::get(Type::getInt64Ty(C), sccIndex);
        auto m = MDNode::get(C, ConstantAsMetadata::get(id));
        indexToIndexMD[sccIndex] = m;
        inst->setMetadata("noelle.pdg.scc.id", m);
      }
    }
  }

  return;
}

} // namespace arcana::noelle
