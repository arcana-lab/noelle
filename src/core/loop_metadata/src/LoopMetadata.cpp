/*
 * Copyright 2019 - 2020 Simone Campanoni
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

#include <fstream>
#include <algorithm>

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"

#include "LoopMetadataPass.hpp"

using namespace llvm;
using namespace llvm::noelle;

std::vector<LoopStructure *> LoopMetadataPass::getLoopStructuresWithoutNoelle(
    Module &M) {
  std::vector<LoopStructure *> loopStructures;
  for (auto &F : M) {
    /*
     * Check if this is application code.
     */
    if (F.empty()) {
      continue;
    }

    /*
     * Check if the function has loops.
     */
    auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    if (std::distance(LI.begin(), LI.end()) == 0) {
      continue;
    }

    /*
     * Consider all loops of the current function.
     */
    auto loops = LI.getLoopsInPreorder();
    for (auto loop : loops) {
      auto loopStructure = new LoopStructure{ loop };
      loopStructures.push_back(loopStructure);
    }
  }

  return loopStructures;
}

bool LoopMetadataPass::setIDs(std::vector<LoopStructure *> &loopStructures) {

  /*
   * Set ID for all loops in the module.
   */
  auto modified = false;
  uint64_t loopID = 0;
  for (auto loopStructure : loopStructures) {
    loopStructure->setID(loopID);
    modified = true;
    errs() << "LOOP_ID " << loopID << "\n";
    loopID++;
  }

  return modified;
}

void LoopMetadataPass::writeLoopIDFile(
    std::vector<LoopStructure *> &loopStructures) {
  /*
   * Create loopIDtoSrcFile.txt
   */
  std::ofstream loopIDtoSrcFile;
  loopIDtoSrcFile.open("loopIDtoSrc.txt");

  for (auto *ls : loopStructures) {
    // We don't need to check if the loop has an ID, we just assigned one to it
    // with setIDs()
    auto loopIDOpt = ls->getID();
    assert(loopIDOpt);
    uint64_t loopID = loopIDOpt.value();

    BasicBlock *header = ls->getHeader();
    Module *M = header->getModule();
    Function *F = header->getParent();
    const std::string &fileName = M->getModuleIdentifier();
    Instruction *terminator = header->getTerminator();
    std::string terminatorAsStr;
    llvm::raw_string_ostream llvmStream(terminatorAsStr);
    llvmStream << *terminator;

    loopIDtoSrcFile << "Loop with ID: " << loopID << "\nin file: " << fileName
                    << "\nat function: " << F->getName().str()
                    << "\nwith header terminator: " << terminatorAsStr
                    << "\n\n";
  }

  loopIDtoSrcFile.close();

  return;
}
