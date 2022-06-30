/*
 * Copyright 2016 - 2022 Kevin McAfee, Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopID.hpp"

namespace llvm::noelle {


LoopID::LoopID()
  :
    ModulePass{ID} 
{

  return ;
}

bool LoopID::doInitialization (Module &M) {
  return false; 
}

bool LoopID::runOnModule (Module &M) {
  errs() << "LoopID: Start\n";

  /*
   * Fetch the outputs of the passes we rely on.
   */
  auto& noelle = getAnalysis<Noelle>();

  /*
   * Fetch the profiles.
   */
  auto profiles = noelle.getProfiles();

  /*
   * Fetch the verbosity level.
   */
  auto verbosity = noelle.getVerbosity();

  /*
   * Fetch all the loops we want to parallelize.
   */
  errs() << "LoopID:  Fetching the program loops\n";
  auto programLoops = noelle.getLoopStructures();
  if (programLoops->size() == 0){
    errs() << "LoopID:    There is no loop to consider\n";

    /*
     * Free the memory.
     */
    delete programLoops;

    errs() << "LoopID: Exit\n";
    return false;
  }

  errs() << "LoopID:    There are " << programLoops->size() << " loops in the program we are going to consider\n";

  uint64_t loopID = 0;
  auto metadataManager = noelle.getMetadataManager();
  for (auto loopStructure : *programLoops){
    if (metadataManager->doesHaveMetadata(loopStructure, "noelle.loop_id.loopid")){
      errs() << "LoopID: loop already has metadata. Abort.\n";
      abort();
    }

    metadataManager->addMetadata(loopStructure, "noelle.loop_id.loopid", std::to_string(loopID));
    loopID += 1;
  }

  /*
   * Free the memory.
   */
  delete programLoops ;

  errs() << "LoopID: Exit\n";

  return false;
}

void LoopID::getAnalysisUsage (AnalysisUsage &AU) const {

  /*
   * Analyses.
   */
  //AU.addRequired<LoopInfoWrapperPass>();

  /*
   * Noelle.
   */
  AU.addRequired<Noelle>();

  return ;
}

}

// Next there is code to register your pass to "opt"
char llvm::noelle::LoopID::ID = 0;
static RegisterPass<LoopID> X("loopid", "Add loop ID as metadata for every loop structure");

// Next there is code to register your pass to "clang"
static LoopID * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
    if(!_PassMaker){ PM.add(_PassMaker = new LoopID());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
    if(!_PassMaker){ PM.add(_PassMaker = new LoopID());}});// ** for -O0
