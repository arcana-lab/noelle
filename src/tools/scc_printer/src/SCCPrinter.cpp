/*
 * Copyright 2016 - 2024 Simone Campanoni
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
#include "SCCPrinter.hpp"
#include "arcana/noelle/core/SCCDAGAttrs.hpp"
#include "arcana/noelle/core/LoopCarriedUnknownSCC.hpp"

namespace arcana::noelle {

static cl::list<int> SCCTypeWhiteList(
    "noelle-scc-printer-white-list",
    cl::ZeroOrMore,
    cl::CommaSeparated,
    cl::desc("Print only a subset of the SCC hierarchy"));
static cl::list<int> SCCTypeBlackList(
    "noelle-scc-printer-black-list",
    cl::ZeroOrMore,
    cl::CommaSeparated,
    cl::desc("Print all but a subset of the SCC hierarchy"));
static cl::opt<bool> PrintSCCInstructions(
    "noelle-scc-printer-insts",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Print all instructions that compose the selected SCCs"));
static cl::opt<bool> PrintLoopIDs(
    "noelle-scc-printer-loops",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Assign and print an incremental ID to loops"));
static cl::opt<int> TargetLoopID("noelle-scc-printer-loop",
                                 cl::ZeroOrMore,
                                 cl::init(-1),
                                 cl::Hidden,
                                 cl::desc("Restrict pass to a single loop"));
static cl::opt<bool> PrintDetails(
    "noelle-scc-printer-details",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Print detailed info about each SCC"));
static cl::opt<std::string> TargetFunctionName(
    "noelle-scc-printer-func",
    cl::init("main"),
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Restrict pass to a single function"));

SCCPrinter::SCCPrinter() : prefix{ "SCCPrinter: " } {

  this->sccTypeWhiteList = SCCTypeWhiteList;
  this->sccTypeBlackList = SCCTypeBlackList;
  this->printSCCInstructions = PrintSCCInstructions;
  this->targetLoopID = TargetLoopID;
  this->targetFunctionName = TargetFunctionName;
  this->loopIDs = PrintLoopIDs;
  this->printDetails = PrintDetails;

  return;
}

PreservedAnalyses SCCPrinter::run(Module &M, llvm::ModuleAnalysisManager &AM) {

  /*
   * Fetch the NOELLE framework.
   */
  auto &noelle = AM.getResult<NoellePass>(M);

  /*
   * Finding the pointer of the given function name
   */
  auto *F = M.getFunction(this->targetFunctionName);

  if (F == nullptr) {
    errs() << this->prefix << "can't find the target function\n";
    return PreservedAnalyses::all();
  }

  if (this->loopIDs) {
    printLoopIDs(noelle.getLoopStructures(F));
    return PreservedAnalyses::all();
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

  return PreservedAnalyses::all();
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

// Next there is code to register your pass to "opt"
llvm::PassPluginLibraryInfo getPluginInfo() {
  return { LLVM_PLUGIN_API_VERSION,
           "scc-printer",
           LLVM_VERSION_STRING,
           [](PassBuilder &PB) {
             /*
              * REGISTRATION FOR "opt -passes='scc-printer'"
              *
              */
             PB.registerPipelineParsingCallback(
                 [](StringRef Name,
                    llvm::ModulePassManager &PM,
                    ArrayRef<llvm::PassBuilder::PipelineElement>) {
                   if (Name == "scc-printer") {
                     PM.addPass(SCCPrinter());
                     return true;
                   }
                   return false;
                 });

             /*
              * REGISTRATION FOR "AM.getResult<NoellePass>()"
              */
             PB.registerAnalysisRegistrationCallback(
                 [](ModuleAnalysisManager &AM) {
                   AM.registerPass([&] { return NoellePass(); });
                 });
           } };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getPluginInfo();
}

} // namespace arcana::noelle
