#include "arcana/noelle/core/NoellePass.hpp"

using namespace arcana::noelle;

namespace {

struct CAT : public PassInfoMixin<CAT> {

  PreservedAnalyses run(Module &M, llvm::ModuleAnalysisManager &AM) {

    /*
     * Fetch NOELLE
     */
    auto &noelle = AM.getResult<NoellePass>(M);
    errs() << "The program has " << noelle.numberOfProgramInstructions()
           << " instructions\n";

    /*
     * Fetch the entry point.
     */
    auto fm = noelle.getFunctionsManager();
    auto mainF = fm->getEntryFunction();

    /*
     * Fetch the data flow engine.
     */
    auto dfe = noelle.getDataFlowEngine();

    /*
     * Define the data flow equations
     */
    auto computeGEN = [](Instruction *i, DataFlowResult *df) {
      if (!isa<LoadInst>(i)) {
        return;
      }
      auto &gen = df->GEN(i);
      gen.insert(i);
      return;
    };
    auto computeKILL = [](Instruction *, DataFlowResult *) { return; };
    auto computeOUT = [](Instruction *inst,
                         Instruction *successor,
                         std::set<Value *> &OUT,
                         DataFlowResult *df) {
      auto &inS = df->IN(successor);
      OUT.insert(inS.begin(), inS.end());
      return;
    };
    auto computeIN =
        [](Instruction *inst, std::set<Value *> &IN, DataFlowResult *df) {
          auto &genI = df->GEN(inst);
          auto &outI = df->OUT(inst);
          IN.insert(outI.begin(), outI.end());
          IN.insert(genI.begin(), genI.end());
          return;
        };

    /*
     * Run the data flow analysis
     */
    auto customDfr = dfe.applyBackward(mainF,
                                       computeGEN,
                                       computeKILL,
                                       computeIN,
                                       computeOUT);

    /*
     * Print
     */
    for (auto &inst : instructions(mainF)) {
      if (!isa<LoadInst>(&inst)) {
        continue;
      }
      auto insts = customDfr->OUT(&inst);
      errs() << " Next are the " << insts.size() << " instructions ";
      errs() << "that could read the value loaded by " << inst << "\n";
      for (auto possibleInst : insts) {
        errs() << "   " << *possibleInst << "\n";
      }
    }

    return PreservedAnalyses::all();
  }
};

// Next there is code to register your pass to "opt"
llvm::PassPluginLibraryInfo getPluginInfo() {
  return { LLVM_PLUGIN_API_VERSION,
           "CAT",
           LLVM_VERSION_STRING,
           [](PassBuilder &PB) {
             /*
              * REGISTRATION FOR "opt -passes='PDGEmbedder'"
              *
              */
             PB.registerPipelineParsingCallback(
                 [](StringRef Name,
                    llvm::ModulePassManager &PM,
                    ArrayRef<llvm::PassBuilder::PipelineElement>) {
                   if (Name == "CAT") {
                     PM.addPass(CAT());
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

} // namespace
