#pragma once

using namespace std;

namespace llvm {

  struct TechniqueWorker {
    std::unordered_map<BasicBlock *, BasicBlock *> basicBlockClones;
    std::unordered_map<Instruction *, Instruction *> instructionClones;
    std::unordered_map<Value *, Value *> liveInClones;
  }
}
