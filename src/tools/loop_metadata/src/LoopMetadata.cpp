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
#include "noelle/core/SystemHeaders.hpp"
#include "llvm/Analysis/LoopInfo.h"

#include "LoopMetadataPass.hpp"

using namespace llvm;
using namespace llvm::noelle;

bool LoopMetadataPass::setIDs(Module &M, Noelle &noelle) {

  /*
   * Fetch all the loops of the program.
   * Min hotness is set to 0.0 to ensure we get all loops.
   */
  auto loopStructures = noelle.getLoopStructures(0.0);

  /*
   * Set ID for all loops in the module.
   */
  auto modified = false;
  auto loopID = 0;
  for (auto loopStructure : *loopStructures) {
    if (loopStructure->doesHaveID()) {
      errs()
          << "LoopID: loop " << *(loopStructure->getHeader()->getTerminator())
          << " already has ID ";
      auto loopID = loopStructure->getID();
      if (loopID) {
        errs() << loopID.value();
      }
      errs() << ". Abort.\n";
      abort();
    }

    loopStructure->setID(loopID);

    modified = true;
    loopID++;
  }

  return modified;
}
