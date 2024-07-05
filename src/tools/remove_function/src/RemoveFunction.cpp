/*
 * Copyright 2021 - 2024  Simone Campanoni
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
#include "RemoveFunction.hpp"

namespace arcana::noelle {

static cl::opt<std::string> NameOfFunctionToDelete(
    "function-name",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Disable the dead code eliminator"));

RemoveFunction::RemoveFunction()
  : functionName{ "" },
    prefix{ "RemoveFunction: " } {

  this->functionName = NameOfFunctionToDelete;

  return;
}

PreservedAnalyses RemoveFunction::run(Module &M,
                                      llvm::ModuleAnalysisManager &AM) {
  errs() << this->prefix << "Start\n";

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto &noelle = AM.getResult<NoellePass>(M);

  /*
   * Fetch the function manager.
   */
  auto fm = noelle.getFunctionsManager();

  /*
   * Fetch the function we want to remove.
   */
  errs() << this->prefix << "  Check if function \"" << this->functionName
         << "\" exists\n";
  auto f = fm->getFunction(this->functionName);
  if (f == nullptr) {
    errs() << this->prefix << "    The function does not exist\n";
    return PreservedAnalyses::all();
  }

  /*
   * Delete the function
   */
  errs() << this->prefix << "    The function exists\n";
  errs() << this->prefix << "  Remove the function\n";
  fm->removeFunction(*f);

  errs() << this->prefix << "Exit\n";
  return PreservedAnalyses::none();
}

// Next there is code to register your pass to "opt"
llvm::PassPluginLibraryInfo getPluginInfo() {
  return { LLVM_PLUGIN_API_VERSION,
           "noelle-rm-function",
           LLVM_VERSION_STRING,
           [](PassBuilder &PB) {
             /*
              * REGISTRATION FOR "opt -passes='noelle-rm-function'"
              *
              */
             PB.registerPipelineParsingCallback(
                 [](StringRef Name,
                    llvm::ModulePassManager &PM,
                    ArrayRef<llvm::PassBuilder::PipelineElement>) {
                   if (Name == "noelle-rm-function") {
                     PM.addPass(RemoveFunction());
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
