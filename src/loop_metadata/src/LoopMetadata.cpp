/*
 * Copyright 2019 - 2020 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"
#include "llvm/Analysis/LoopInfo.h"

#include "LoopMetadataPass.hpp"

using namespace llvm;

bool LoopMetadataPass::tagLoops (
  LLVMContext &context,
  Module &M,
  Noelle &par
  ){

  /*
   * Fetch all the loops of the program.
   */
  auto loopStructuresToParallelize = par.getLoopStructures(0.0);

  /*
   * Tag all loops.
   */
  auto modified = false;
  auto loopID = 0;
  for (auto loopStructure : *loopStructuresToParallelize){

    /*
     * We cannot attach metadata to loops in the current LLVM infrastructure.
     * We cannot attach metadata to basic blocks in the current LLVM infrastructure.
     * Hence, we attach metadata to the terminator of the header of the loop to represent the metadata of the loop.
     * 
     * Fetch the header.
     */
    auto header = loopStructure->getHeader();

    /*
     * Fetch the terminator.
     */
    auto headerTerminator = header->getTerminator();

    /*
     * Tag the header terminator to tag the loop as "to be optimized"
     */
    auto trueMetadataString = MDString::get(context, "true");
    auto trueMetadata = MDNode::get(context, trueMetadataString);
    headerTerminator->setMetadata("noelle.loop_optimize", trueMetadata);

    /*
     * Tag the header terminator to make explicit the ID of the loop.
     */
    auto loopIDString = std::to_string(loopID);
    auto loopIDMetadataAsString = MDString::get(context, loopIDString);
    auto loopIDMetadata = MDNode::get(context, loopIDMetadataAsString);
    headerTerminator->setMetadata("noelle.loop_ID", loopIDMetadata);

    /*
     * Remember that we have modified the code.
     */
    modified = true ;
    loopID++;
  }

  return modified;
}
