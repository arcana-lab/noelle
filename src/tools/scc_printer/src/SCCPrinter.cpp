/*
 * Copyright 2016 - 2022 Simone Campanoni
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
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY << " =
 " << FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM <<
 " = " << DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <string>
#include <unordered_map>
#include <vector>

#include "arcana/noelle/core/SCCDAGAttrs.hpp"
#include "arcana/noelle/core/LoopCarriedUnknownSCC.hpp"

#include "SCCPrinter.hpp"

using namespace std;

namespace arcana::noelle {

SCCPrinter::SCCPrinter() : ModulePass{ ID }, prefix{ "SCCPrinter: " } {
  return;
}

bool SCCPrinter::runOnModule(Module &M) {
  auto &noelle = getAnalysis<Noelle>();

  /*
   * Finding the pointer of the given function name
   */
  auto *F = M.getFunction(this->targetFunctionName);

  if (F == nullptr) {
    errs() << this->prefix << "can't find the target function\n";
    return false;
  }

  if (this->loopIDs) {
    printLoopIDs(noelle.getLoopStructures(F));
    return false;
  }

  if (this->targetLoopID < 0) {
    errs() << this->prefix << "please specify a loop ID\n";
    return false;
  }

  const auto isSelected = [&](GenericSCC::SCCKind t) {
    const auto &WL = this->sccTypeWhiteList;
    const auto &BL = this->sccTypeBlackList;
    if (!WL.empty()) {
      return std::find(WL.begin(), WL.end(), t) != std::end(WL);
    }
    if (!BL.empty()) {
      return std::find(BL.begin(), BL.end(), t) == std::end(BL);
    }

    /*
     * If no type has been specified we will select them all
     */
    return true;
  };

  auto LSs = noelle.getLoopStructures();
  LoopStructure *LS = nullptr;
  for (auto *x : *LSs) {
    if (x->getID().value() == this->targetLoopID) {
      LS = x;
    }
  }
  assert(LS);

  auto LC = noelle.getLoopContent(LS);
  auto sccManager = LC->getSCCManager();
  auto SCCNodes = sccManager->getSCCDAG()->getSCCs();

  uint64_t id = 0;
  for (auto sccNode : SCCNodes) {
    auto scc = sccManager->getSCCAttrs(sccNode);
    auto type = scc->getKind();
    if (isSelected(type)) {
      printSCC(scc);
    }
    id++;
  }

  return false;
}

void SCCPrinter::printSCC(GenericSCC *scc) {
  auto sccNode = scc->getSCC();
  auto type = scc->getKind();

  errs() << this->prefix << "Found \e[1;32m" << getSCCTypeName(scc->getKind())
         << "\e[0m (Type ID " << type << ")\n";

  if (this->printSCCInstructions) {
    errs() << this->prefix << "  \e[32mInstructions\e[0m: \n";
    for (auto *I : sccNode->getInstructions()) {
      errs() << *I << "\n";
    }
    errs() << "\n";
  }

  if (this->printDetails) {
    errs() << this->prefix << "  \e[32mDetails\e[0m: \n";
    sccNode->print(errs(), "");
  }
}

void SCCPrinter::printLoopIDs(std::vector<LoopStructure *> *LSs) {
  errs() << this->prefix << "Selected function: \e[35m"
         << this->targetFunctionName << "\e[0m\n";
  for (auto LS : *LSs) {
    auto id = LS->getID().value();
    errs() << this->prefix << "\e[1;32mLoop ID " << id << "\e[0m:\n";
    errs() << *LS->getHeader() << "\n";
  }
}

std::string getSCCTypeName(GenericSCC::SCCKind type) {
  switch (type) {
    case GenericSCC::LOOP_CARRIED:
      return "LOOP_CARRIED";
    case GenericSCC::REDUCTION:
      return "REDUCTION";
    case GenericSCC::BINARY_REDUCTION:
      return "BINARY_REDUCTION";
    case GenericSCC::LAST_REDUCTION:
      return "LAST_REDUCTION";
    case GenericSCC::RECOMPUTABLE:
      return "RECOMPUTABLE";
    case GenericSCC::SINGLE_ACCUMULATOR_RECOMPUTABLE:
      return "SINGLE_ACCUMULATOR_RECOMPUTABLE";
    case GenericSCC::INDUCTION_VARIABLE:
      return "INDUCTION_VARIABLE";
    case GenericSCC::LINEAR_INDUCTION_VARIABLE:
      return "LINEAR_INDUCTION_VARIABLE";
    case GenericSCC::LAST_INDUCTION_VARIABLE:
      return "LAST_INDUCTION_VARIABLE";
    case GenericSCC::PERIODIC_VARIABLE:
      return "PERIODIC_VARIABLE";
    case GenericSCC::LAST_SINGLE_ACCUMULATOR_RECOMPUTABLE:
      return "LAST_SINGLE_ACCUMULATOR_RECOMPUTABLE";
    case GenericSCC::UNKNOWN_CLOSED_FORM:
      return "UNKNOWN_CLOSED_FORM";
    case GenericSCC::LAST_RECOMPUTABLE:
      return "LAST_RECOMPUTABLE";
    case GenericSCC::MEMORY_CLONABLE:
      return "MEMORY_CLONABLE";
    case GenericSCC::STACK_OBJECT_CLONABLE:
      return "STACK_OBJECT_CLONABLE";
    case GenericSCC::LAST_MEMORY_CLONABLE:
      return "LAST_MEMORY_CLONABLE";
    case GenericSCC::LOOP_CARRIED_UNKNOWN:
      return "LOOP_CARRIED_UNKNOWN";
    case GenericSCC::LAST_LOOP_CARRIED:
      return "LAST_LOOP_CARRIED";
    case GenericSCC::LOOP_ITERATION:
      return "LOOP_ITERATION";
    case GenericSCC::LAST_LOOP_ITERATION:
      return "LAST_LOOP_ITERATION";
    default:
      assert(false);
  }
}

} // namespace arcana::noelle
