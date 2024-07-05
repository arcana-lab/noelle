/*
 * Copyright 2019 - 2024  Simone Campanoni
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
#include "DeadFunctionEliminator.hpp"

namespace arcana::noelle {

static cl::opt<bool> DisableDead("noelle-disable-dead",
                                 cl::ZeroOrMore,
                                 cl::Hidden,
                                 cl::desc("Disable the dead code eliminator"));

DeadFunctionEliminator::DeadFunctionEliminator()
  : enableTransformation{ true },
    prefix{ "DeadFunctionEliminator: " } {

  if (DisableDead.getNumOccurrences() > 0) {
    this->enableTransformation = false;
  }

  return;
}

PreservedAnalyses DeadFunctionEliminator::run(Module &M,
                                              llvm::ModuleAnalysisManager &AM) {

  /*
   * Check if the transformation is enabled.
   */
  if (!this->enableTransformation) {
    return PreservedAnalyses::all();
  }
  auto modified = false;
  errs() << this->prefix << "Start\n";

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto &noelle = AM.getResult<NoellePass>(M);

  /*
   * Fetch the call graph.
   */
  auto fm = noelle.getFunctionsManager();
  auto pcg = fm->getProgramCallGraph();

  /*
   * Check if there are functions with only one caller.
   * Inline them.
   */
  errs()
      << this->prefix
      << "  Inline functions that are invoked only by one call instruction and that do not escape into memory\n";
  for (auto node : pcg->getFunctionNodes()) {

    /*
     * Fetch the node.
     */
    auto nodeFunction = node->getFunction();

    /*
     * Check the function has a body.
     */
    if (nodeFunction->empty()) {
      continue;
    }

    /*
     * Check how many other functions can invoke @nodeFunction
     */
    auto callerNodes = pcg->getIncomingEdges(node);
    if (callerNodes.size() != 1) {
      continue;
    }

    /*
     * Check if the function can escape.
     */
    if (pcg->canFunctionEscape(nodeFunction)) {
      continue;
    }

    /*
     * We found a function that has only one other function that can invoke it.
     *
     * Check how many call instructions can invoke @node
     */
    auto callingEdge = *callerNodes.begin();
    auto callerEdges = callingEdge->getSubEdges();
    if (callerEdges.size() != 1) {
      continue;
    }

    /*
     * @node has only one caller.
     *
     * Fetch the caller.
     */
    auto callerEdge = *callerEdges.begin();
    auto callerNode = callerEdge->getCaller();
    auto callerInst = callerNode->getInstruction();

    /*
     * Check the single caller isn't part of the same function of @node.
     */
    if (callerInst->getFunction() == nodeFunction) {
      continue;
    }

    /*
     * Check the instruction isn't an invoke.
     */
    if (isa<InvokeInst>(callerInst)) {

      /*
       * We cannot inline invoke instructions to preserve the exception handling
       * mechanisms.
       */
      continue;
    }

    /*
     * We can inline @nodeFunction without increasing the size of the binary.
     *
     * Check if we need to translate an indirect call to a direct call.
     */
    auto callInst = cast<CallInst>(callerInst);
    if (callInst->getCalledFunction() == nullptr) {

      /*
       * This is an indirect call.
       * Translate it to a direct call.
       */
      errs() << this->prefix << "    Found an opportunity to devirtualize\n";
      // TODO
      continue;
    }
    assert(callInst->getCalledFunction() == nodeFunction);
    errs() << this->prefix << "    Inline " << *callInst << " into "
           << callInst->getFunction()->getName() << "\n";
    InlineFunctionInfo IFI;
    auto inlineResult = InlineFunction(*callInst, IFI);
    if (inlineResult.isSuccess()) {
      modified = true;
    }
  }
  if (modified) {
    errs() << this->prefix << "Exit\n";
    return PreservedAnalyses::none();
  }

  /*
   * Fetch the islands.
   */
  errs() << this->prefix << "  Get the islands\n";
  auto islands = pcg->getIslands();

  /*
   * Fetch the island of the entry method of the program.
   */
  errs() << this->prefix
         << "  Identify the islands reachable from the entry points\n";
  auto entryF = fm->getEntryFunction();
  auto entryIsland = islands[entryF];
  std::unordered_set<CallGraph *> liveIslands{ entryIsland };

  /*
   * Fetch the islands of all constructors.
   */
  auto ctors = fm->getProgramConstructors();
  for (auto ctor : ctors) {
    auto ctorIsland = islands[ctor];
    assert(ctorIsland != nullptr);
    liveIslands.insert(ctorIsland);
  }
  for (auto island : liveIslands) {
    errs() << this->prefix << "    Island\n";

    /*
     * Sort the functions using their pointers.
     * This guarantee determinism because the pointers reflect their position in
     * the bitcode file.
     */
    std::vector<Function *> sortedNodes;
    for (auto node : island->getFunctionNodes()) {
      auto f = node->getFunction();
      sortedNodes.push_back(f);
    }
    std::sort(sortedNodes.begin(), sortedNodes.end());

    /*
     * Print the functions
     */
    for (auto f : sortedNodes) {
      errs() << this->prefix << "      " << f->getName() << "\n";
    }
  }

  /*
   * Delete dead functions.
   */
  errs() << this->prefix << "  Identify the functions that can be deleted\n";
  std::vector<Function *> toDelete;
  for (auto &F : M) {

    /*
     * Check if &F is dead
     */
    if (F.isIntrinsic()) {
      continue;
    }
    if (F.empty()) {
      continue;
    }
    if (liveIslands.find(islands[&F]) != liveIslands.end()) {
      continue;
    }
    if (pcg->canFunctionEscape(&F)) {
      continue;
    }

    errs() << this->prefix << "    Function " << F.getName() << " is dead\n";
    toDelete.push_back(&F);
  }
  for (auto f : toDelete) {
    f->eraseFromParent();
    modified = true;
  }

  errs() << this->prefix << "Exit\n";
  if (modified) {
    return PreservedAnalyses::none();
  }
  return PreservedAnalyses::all();
}

// Next there is code to register your pass to "opt"
llvm::PassPluginLibraryInfo getPluginInfo() {
  return { LLVM_PLUGIN_API_VERSION,
           "DeadFunctionEliminator",
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
                   if (Name == "DeadFunctionEliminator") {
                     PM.addPass(DeadFunctionEliminator());
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
