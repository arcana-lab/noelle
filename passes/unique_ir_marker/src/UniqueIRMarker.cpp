
#include <UniqueIRMarker.hpp>

#include "UniqueIRMarker.hpp"

using std::addressof;

void UniqueIRMarker::visitModule(Module &M) {
  M.print(errs(), nullptr);
  auto metaNode = M.getOrInsertNamedMetadata(UniqueIRConstants::VIAModule);

  auto moduleUID = uniqueModuleCounter();
  auto *meta = buildNode(M.getContext(), moduleUID);
  if (metaNode->getNumOperands() == 0) {
    metaNode->addOperand(meta);
  } else if (metaNode->getNumOperands() == 1) {
    AlreadyMarked = true;
    auto setModuleID = UniqueIRMarkerReader::getModuleID(addressof(M));
    assert(setModuleID && setModuleID.value() >= 0);
  } else {
    assert (0 && "Must either have not operands or 1");
  }


}


void UniqueIRMarker::visitFunction(Function &F) {
  if (AlreadyMarked) return;
  LLVMContext& Context = F.getContext();
  auto *countMeta = buildNode(Context, FunctionCounter++);
  F.setMetadata(UniqueIRConstants::VIAFunction, countMeta);

  if ( F.empty() ) return;

  auto &LoopInfo  = MP.getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();

  for ( auto &Loop : LoopInfo.getLoopsInPreorder() ) {
    SmallVector<Metadata *, 2> MDs;
    MDs.push_back(ConstantAsMetadata::get(ConstantInt::get(Context, llvm::APInt(IDSize, 0, false))));
    MDs.push_back(ConstantAsMetadata::get(ConstantInt::get(Context, llvm::APInt(IDSize, LoopCounter++, false))));
    auto *node = MDNode::get(Context, MDs);
    node->replaceOperandWith(0,node);
    Loop->setLoopID(node);
  }
}

void UniqueIRMarker::visitBasicBlock(BasicBlock &BB) {
  if (AlreadyMarked) return;
  if( BB.empty() ) return;
  auto *countMeta = buildNode(BB.getContext(), BasicBlockCounter++);
  BB.front().setMetadata(UniqueIRConstants::VIABasicBlock, countMeta);
}

void UniqueIRMarker::visitInstruction(Instruction &I) {
  if (AlreadyMarked) return;
  auto *countMeta = buildNode(I.getContext(), uniqueInstructionCounter());
  I.setMetadata(UniqueIRConstants::VIAInstruction, countMeta);
}

MDNode *UniqueIRMarker::buildNode(LLVMContext& C, IDType value) {
  return MDNode::get(C, ConstantAsMetadata::get(ConstantInt::get(C, llvm::APInt(IDSize, value, false))));
}

UniqueIRMarker::UniqueIRMarker(ModulePass& MP) :
  MP(MP), InstructionCounter(0), FunctionCounter(0), BasicBlockCounter(0), LoopCounter(0), ModuleCounter(0) {}

uint64_t UniqueIRMarker::uniqueInstructionCounter() {
  // uint64_t max is reserved for null.
  assert(InstructionCounter <= (std::numeric_limits<uint64_t>::max() - 1) && "InstructionCounter has overrun" );
  return InstructionCounter++;
}

IDType UniqueIRMarker::uniqueModuleCounter() {
  assert(ModuleCounter <= (std::numeric_limits<uint64_t>::max() - 1) && "ModuleCounter has overrun" );
  return ModuleCounter++;
}

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
    return false;
  }

  UniqueIRMarker walker{*this};

  walker.visit(M);

  // Since metadata will always be changed...
  return false;
}

// register pass

char UniqueIRMarkerPass::ID = 0;
static RegisterPass<UniqueIRMarkerPass> X("UniqueIRID", "Add a unique ID to Instructions, BB, Loop, Function and Modules "
                                                        "which will be used by VIA.");
