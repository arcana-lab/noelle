#pragma once

using namespace llvm;

namespace llvm {

  struct ChunkInfo {
    Function *chunker;

    BasicBlock *entryBlock, *exitBlock;
  };
 
}
