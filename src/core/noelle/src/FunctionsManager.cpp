/*
 * Copyright 2020 - 2021  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/FunctionsManager.hpp"

namespace llvm::noelle {
      
FunctionsManager::FunctionsManager (Module &m, PDGAnalysis &noellePDGAnalysis)
  : 
    program{m}
    , pdgAnalysis{noellePDGAnalysis}
    , pcg{nullptr}
{
  return ;
}

Function * FunctionsManager::getEntryFunction (void) const {
  auto f = this->program.getFunction("main");
  return f;
}

CallGraph * FunctionsManager::getProgramCallGraph (void) {
  if (this->pcg == nullptr){
    this->pcg = this->pdgAnalysis.getProgramCallGraph();
  }

  return this->pcg;
}

bool FunctionsManager::isTheLibraryFunctionPure (Function *libraryFunction){

  /*
   * Check if the function is a library function.
   */
  if (libraryFunction == nullptr){
    return false;
  }
  if (!libraryFunction->empty()){
    return false;
  }
  
  /*
   * The function is a library function.
   * Check if it is known to be pure.
   */
  if (this->pdgAnalysis.isTheLibraryFunctionPure(libraryFunction)){
    return true;
  }

  return false;
}

}
