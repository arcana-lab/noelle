#pragma once

#include "HELIX.hpp"
#include "SCC.hpp"

namespace llvm {

  class SequentialSegment {
    public:
      SequentialSegment (LoopDependenceInfo *LDI, SCC *scc) ;

  };

}
