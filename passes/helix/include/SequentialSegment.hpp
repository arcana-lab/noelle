/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "HELIX.hpp"
#include "SCCDAGPartition.hpp"
#include "llvm/ADT/iterator_range.h"

namespace llvm {

  class SequentialSegment {
    public:
      SequentialSegment (LoopDependenceInfo *LDI, SCCset *sccs, int32_t ID, Verbosity verbosity) ;

      void forEachEntry (std::function <void (Instruction *justAfterEntry)> whatToDo);

      void forEachExit (std::function <void (Instruction *justBeforeExit)> whatToDo);

      int32_t getID (void);

      iterator_range<SCCset::iterator>
      getSCCs() { return make_range(sccs->begin(), sccs->end()); }

    private:
      std::set<Instruction *> entries;
      std::set<Instruction *> exits;
      SCCset *sccs;
      int32_t ID;
      Verbosity verbosity;
  };

}
