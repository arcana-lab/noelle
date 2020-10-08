#pragma once

#include "SystemHeaders.hpp"

#include "UniqueIRConstants.hpp"
#include "UniqueIRMarkerReader.hpp"

using namespace llvm;

namespace llvm::noelle {

  class UniqueIRVerifier : public InstVisitor<UniqueIRVerifier> {

  public:
    explicit UniqueIRVerifier(ModulePass& MP);

    void visitModule     (Module &M);
    void visitFunction   (Function &F);
    void visitInstruction(Instruction &I);

    constexpr static unsigned int IDSize = sizeof(IDType) * 8; // IDs are in bits

    bool verifyLoops();

  private:
    ModulePass& MP;

    // if the bitcode file already has a module 'VIA.M.ID' definition then this walker will instead verify that
    // the metadata is correctly inserted.
    bool AlreadyMarked = false;

    std::set<IDType> LoopIDsFromLoopInfo;
    std::set<IDType> LoopIDsFromPreviousMarkerPass;

    void checkFunction(Function &);
    void checkInstruction(Instruction &);
  };

}