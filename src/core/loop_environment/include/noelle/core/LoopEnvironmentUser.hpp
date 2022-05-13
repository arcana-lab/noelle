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

    void setEnvArray (Value *envArr) { this->envArray = envArr; }

    void createEnvPtr (IRBuilder<> b, int envIndex, Type *type);

    void createReducableEnvPtr (
      IRBuilder<> b,
      int envIndex,
      Type *type,
      int reducerCount,
      Value *reducerIndV
    );

    void addLiveInIndex (uint32_t ind) ;
    void addLiveOutIndex (uint32_t ind) ;

    iterator_range<std::set<int>::iterator> getEnvIndicesOfLiveInVars() { 
      return make_range(liveInInds.begin(), liveInInds.end());
    }
    iterator_range<std::set<int>::iterator> getEnvIndicesOfLiveOutVars() { 
      return make_range(liveOutInds.begin(), liveOutInds.end());
    }

    Instruction * getEnvPtr (uint32_t ind);

    ~LoopEnvironmentUser ();

   private:
    Value *envArray;

		/*
		 * Maps from environment index to load/stores
		 */
    std::unordered_map<int, Instruction *> envIndexToPtr;
    std::set<int> liveInInds;
    std::set<int> liveOutInds;
  };

}
