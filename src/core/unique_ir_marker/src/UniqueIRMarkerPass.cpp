#include <UniqueIRMarker.hpp>
#include <UniqueIRVerifier.hpp>
#include "UniqueIRMarkerPass.hpp"

using namespace llvm;

cl::opt<bool> InstrumentModule("instrument-ir");
cl::opt<bool> ReinstrumentModule("reinstrument-ir");
cl::opt<bool> RenumberModule("renumber-ir");
cl::opt<bool> VerifyModule("verify-ir");

namespace llvm::noelle {

  UniqueIRMarkerPass::UniqueIRMarkerPass() : ModulePass(ID) {}

  bool UniqueIRMarkerPass::doInitialization(Module &M) {
    return true;
  }

  void UniqueIRMarkerPass::getAnalysisUsage(AnalysisUsage& AU) const {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.setPreservesAll();
  }

  bool UniqueIRMarkerPass::runOnModule(Module &M) {

    if( M.empty() ) {
      return true;
    }

    if (!(InstrumentModule || ReinstrumentModule || RenumberModule))
      return true;

    auto mode = InstrumentModule ? MarkerMode::Instrument :
        (ReinstrumentModule ? MarkerMode::Reinstrument : MarkerMode::Renumber);

    if (InstrumentModule || ReinstrumentModule || RenumberModule) {
      UniqueIRMarker walker{*this, mode };
      walker.visit(M);
      return false;
    } else if (VerifyModule) {
      UniqueIRVerifier walker{*this};
      walker.visit(M);
      assert(walker.verifyLoops() && "UniqueIRMarker -- Error: "
                                    "The verify failed for UniqueIRMarker which means that the IR loops have changed "
                                    "the metadata reflecting this change.");
    }
    return true;
  }

  // register pass
  char UniqueIRMarkerPass::ID = 0;
  static RegisterPass<UniqueIRMarkerPass> X("UniqueIRID", "Add a unique ID to Instructions, BB, Loop, Function and Modules "
                                                          "which will be used by VIA.");

}