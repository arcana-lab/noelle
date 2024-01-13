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
#include <unordered_map>

#include "noelle/core/SCCDAGAttrs.hpp"
#include "noelle/core/LoopCarriedUnknownSCC.hpp"

#include "SCCPrinter.hpp"

namespace arcana::noelle {

SCCPrinter::SCCPrinter() : ModulePass{ ID }, prefix{ "Noelle: SCCPrinter: " } {
  return;
}

bool SCCPrinter::runOnModule(Module &M) {
  auto &noelle = getAnalysis<Noelle>();

  if (this->types) {
    printTypes();
    return false;
  }

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
  auto LS = (*LSs)[this->targetLoopID];

  auto LC = noelle.getLoopContent(LS);
  auto sccManager = LC->getSCCManager();
  auto SCCNodes = sccManager->getSCCDAG()->getSCCs();

  if (this->targetSCCID >= 0) {
    if (this->targetSCCID >= SCCNodes.size()) {
      errs() << this->prefix << "invalid SCC ID\n";
      return false;
    }
    int idx = 0;
    auto target = SCCNodes.begin();
    std::advance(target, this->targetSCCID);
    auto scc = sccManager->getSCCAttrs(*target);
    printSCC(this->targetSCCID, scc, /*inDetail=*/true);
    return false;
  }

  int id = 0;
  for (auto sccNode : SCCNodes) {
    auto scc = sccManager->getSCCAttrs(sccNode);
    auto type = scc->getKind();
    if (isSelected(type)) {
      printSCC(id, scc, /*inDetail=*/false);
    }
    id++;
  }
  return false;
}

void SCCPrinter::printSCC(int id, GenericSCC *scc, bool inDetail) {
  auto sccNode = scc->getSCC();

  errs() << this->prefix << "SCCID." << id << " "
         << "TypeID." << scc->getKind() << " = "
         << getSCCTypeName(scc->getKind()) << "\n";

  if (this->printSCCInstructions) {
    for (auto *I : sccNode->getInstructions()) {
      errs() << this->prefix << *I << "\n";
    }
    errs() << this->prefix << "\n";
  }

  if (inDetail) {
    sccNode->print(errs(), this->prefix);
  }
}

void SCCPrinter::printTypes() {
  errs() << this->prefix << "List of SCC types and their ID\n";

  auto print = [prefix = this->prefix](auto type) {
    errs()
        << prefix << "TypeID." << type << " = " << getSCCTypeName(type) << "\n";
  };

  print(GenericSCC::LOOP_CARRIED);
  print(GenericSCC::REDUCTION);
  print(GenericSCC::BINARY_REDUCTION);
  print(GenericSCC::LAST_REDUCTION);
  print(GenericSCC::RECOMPUTABLE);
  print(GenericSCC::SINGLE_ACCUMULATOR_RECOMPUTABLE);
  print(GenericSCC::INDUCTION_VARIABLE);
  print(GenericSCC::LINEAR_INDUCTION_VARIABLE);
  print(GenericSCC::LAST_INDUCTION_VARIABLE);
  print(GenericSCC::PERIODIC_VARIABLE);
  print(GenericSCC::LAST_SINGLE_ACCUMULATOR_RECOMPUTABLE);
  print(GenericSCC::UNKNOWN_CLOSED_FORM);
  print(GenericSCC::LAST_RECOMPUTABLE);
  print(GenericSCC::MEMORY_CLONABLE);
  print(GenericSCC::STACK_OBJECT_CLONABLE);
  print(GenericSCC::LAST_MEMORY_CLONABLE);
  print(GenericSCC::LOOP_CARRIED_UNKNOWN);
  print(GenericSCC::LAST_LOOP_CARRIED);
  print(GenericSCC::LOOP_ITERATION);
  print(GenericSCC::LAST_LOOP_ITERATION);
}

void SCCPrinter::printLoopIDs(std::vector<LoopStructure *> *LSs) {
  int id = 0;
  for (auto LS : *LSs) {
    errs() << this->prefix << "Loop ID = " << id << ":\n";
    for (auto &I : *LS->getHeader()) {
      errs() << this->prefix << I << "\n";
    }
    id++;
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
