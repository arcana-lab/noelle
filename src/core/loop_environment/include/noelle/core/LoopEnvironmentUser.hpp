/*
 * Copyright 2021 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/SystemHeaders.hpp"

namespace llvm::noelle {

  class LoopEnvironmentUser {
   public:
    LoopEnvironmentUser ();

    void setEnvironmentArray (
      Value *envArr
      ) ;

    Instruction * createEnvironmentVariablePointer (
      IRBuilder<> b, 
      uint32_t envIndex, 
      Type *type
      );

    void createReducableEnvPtr (
      IRBuilder<> b,
      uint32_t envIndex,
      Type *type,
      uint32_t reducerCount,
      Value *reducerIndV
    );

    void addLiveInIndex (uint32_t ind) ;

    void addLiveOutIndex (uint32_t ind) ;

    iterator_range<std::set<uint32_t>::iterator> getEnvIndicesOfLiveInVars(void);

    iterator_range<std::set<uint32_t>::iterator> getEnvIndicesOfLiveOutVars (void);

    Instruction * getEnvPtr (uint32_t ind);

    ~LoopEnvironmentUser ();

   private:
    Value *envArray;

		/*
		 * Maps from environment index to load/stores
		 */
    std::unordered_map<uint32_t, Instruction *> envIndexToPtr;
    std::set<uint32_t> liveInInds;
    std::set<uint32_t> liveOutInds;
  };

}
