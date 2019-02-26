
#include "UniqueIRMarker.hpp"


void UniqueIRMarker::visitModule(Module &M) {
  auto metaNode = M.getOrInsertNamedMetadata(VIAModule);

  metaNode->addOperand(buildNode(M.getContext(), ModuleCounter++));
}


void UniqueIRMarker::visitFunction(Function &F) {
  LLVMContext& Context = F.getContext();
  auto* countMeta = buildNode(Context, FunctionCounter++);
  F.setMetadata(VIAFunction, countMeta);

  auto& LoopInfo  = MP.getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();

  for ( auto &Loop : LoopInfo.getLoopsInPreorder() ) {
    SmallVector<Metadata *, 2> MDs;
    MDs.push_back(ConstantAsMetadata::get(ConstantInt::get(Context, llvm::APInt(IDSize, 0, false))));
    MDs.push_back(ConstantAsMetadata::get(ConstantInt::get(Context, llvm::APInt(IDSize, LoopCounter++, false))));
    auto* node = MDNode::get(Context, MDs);
    node->replaceOperandWith(0,node);
    Loop->setLoopID(node);
  }
}

void UniqueIRMarker::visitBasicBlock(BasicBlock &BB) {
  if( BB.empty() ) return;
  auto* countMeta = buildNode(BB.getContext(), BasicBlockCounter++);
  BB.front().setMetadata(VIABasicBlock, countMeta);
}

void UniqueIRMarker::visitInstruction(Instruction &I) {
  auto* countMeta = buildNode(I.getContext(), uniqueInstructionCounter());
  I.setMetadata(VIAInstruction, countMeta);
}

MDNode *UniqueIRMarker::buildNode(LLVMContext& C, IDType value) {
  return MDNode::get(C, ConstantAsMetadata::get(ConstantInt::get(C, llvm::APInt(IDSize, value, false))));
}

UniqueIRMarker::UniqueIRMarker(ModulePass& MP) :
  MP(MP), InstructionCounter(0), FunctionCounter(0), BasicBlockCounter(0), LoopCounter(0), ModuleCounter(0) {}

uint64_t UniqueIRMarker::uniqueInstructionCounter() {
  assert(InstructionCounter <= std::numeric_limits<uint64_t>::max() && "InstructionCounter has overrun" );
  return InstructionCounter++;
}


UniqueIRMarkerPass::UniqueIRMarkerPass() : ModulePass(ID) {}

bool UniqueIRMarkerPass::doInitialization(Module &M) {
  return true;
}

void UniqueIRMarkerPass::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addPreserved<LoopInfoWrapperPass>();
}

bool UniqueIRMarkerPass::runOnModule(Module &M) {

  if( M.empty() ) {
    return false;
  }

  UniqueIRMarker walker{*this};

  walker.visit(M);

  // Since metadata will always be changed...
  return true;
}

// register pass

char UniqueIRMarkerPass::ID = 0;
static RegisterPass<UniqueIRMarkerPass> X("UniqueIRID", "Add a unique ID to Instructions, BB, Loop, Function and Modules "
                                                        "which will be used by VIA.");
