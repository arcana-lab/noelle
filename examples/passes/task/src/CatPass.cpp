#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "arcana/noelle/core/NoellePass.hpp"
#include "arcana/noelle/core/Task.hpp"

using namespace arcana::noelle;

namespace {

class MyTask : public Task {
public:
  MyTask(FunctionType *taskSignature, Module &M) : Task(taskSignature, M) {
    auto argIter = this->F->arg_begin();
    this->envArg = (Value *)&*(argIter++);
  }
};

struct CAT : public ModulePass {
  static char ID;

  CAT() : ModulePass(ID) {}

  bool doInitialization(Module &M) override {
    return false;
  }

  bool runOnModule(Module &M) override {

    /*
     * Fetch NOELLE
     */
    auto &noelle = getAnalysis<NoellePass>().getNoelle();

    /*
     * Check if we have profiles.
     */
    auto hot = noelle.getProfiles();
    if (!hot->isAvailable()) {
      errs() << "The profiler isn't available\n";
      return false;
    }

    /*
     * Fetch the loops with all their abstractions
     * (e.g., loop dependence graph, sccdag)
     */
    auto loops = noelle.getLoopStructures();
    if (loops->size() == 0) {
      errs() << "The program has no loops\n";
      return false;
    }

    /*
     * Fetch the hottest loop that has a single successor.
     */
    noelle.sortByHotness(*loops);
    LoopContent *hottestLoop = nullptr;
    for (auto tmpLoop : *loops) {

      /*
       * This example only handles loops with a single successor.
       * Hence, loops with a single exit basic block.
       */
      if (tmpLoop->numberOfExitBasicBlocks() == 1) {

        /*
         * This example only handles loops with no live-outs.
         */
        auto tmpLoopContent = noelle.getLoopContent(tmpLoop);
        auto env = tmpLoopContent->getEnvironment();
        if (env->getNumberOfLiveOuts() == 0) {
          hottestLoop = tmpLoopContent;
          break;
        }

        delete tmpLoopContent;
      }
    }
    if (hottestLoop == nullptr) {
      errs() << "There is no loop that we can handle.\n";
      return false;
    }
    auto hottestLoopStructure = hottestLoop->getLoopStructure();
    auto hottestLoopFunction = hottestLoopStructure->getFunction();
    auto entryInst = hottestLoopStructure->getEntryInstruction();
    errs() << "Hottest loop " << *entryInst << "\n";
    errs() << "  Function = " << hottestLoopFunction->getName() << "\n";
    errs() << "  Nesting level = " << hottestLoopStructure->getNestingLevel()
           << "\n";

    /*
     * Define the signature of the task.
     */
    auto tm = noelle.getTypesManager();
    auto funcArgTypes = ArrayRef<Type *>({ tm->getVoidPointerType() });
    auto taskSignature =
        FunctionType::get(tm->getVoidType(), funcArgTypes, false);

    /*
     * Get the environment of the target loop.
     */
    auto env = hottestLoop->getEnvironment();

    /*
     * Create an empty task.
     */
    auto t = new MyTask(taskSignature, M);
    auto entryBlockOfTaskBody = t->getEntry();

    /*
     * Record the mapping from the pre-header of the original loop to the one of
     * the loop within the task.
     */
    auto loopPreHeader = hottestLoopStructure->getPreHeader();
    t->addBasicBlock(loopPreHeader, entryBlockOfTaskBody);

    /*
     * Generate the code to allocate and initialize the loop environment that
     * will be given to the task as its input. This code will be in the callers
     * of the task. In our example, we'll have a single caller.
     */
    auto program = noelle.getProgram();
    auto envBuilder = new LoopEnvironmentBuilder(program->getContext(), env, 1);
    auto envUser = envBuilder->getUser(0);

    /*
     * Generate code within the task to cast the pointer of the environment
     * given as input.
     */
    IRBuilder<> entryBuilder(entryBlockOfTaskBody);
    auto bitcastInst = entryBuilder.CreateBitCast(
        t->getEnvironment(),
        PointerType::getUnqual(envBuilder->getEnvironmentArrayType()));
    envUser->setEnvironmentArray(bitcastInst);

    /*
     * Define the body of the task.
     */
    t->cloneAndAddBasicBlocks(hottestLoopStructure->getBasicBlocks());

    /*
     * Declare the live-in and live-out variables of the task using the caller
     * variables.
     */
    for (auto envID : env->getEnvIDsOfLiveInVars()) {
      envUser->addLiveIn(envID);
    }
    for (auto envID : env->getEnvIDsOfLiveOutVars()) {
      envUser->addLiveOut(envID);
    }

    /*
     * Add the jump to the loop within the task from the entry block of the
     * task.
     */
    auto loopHeader = hottestLoopStructure->getHeader();
    auto headerClone = t->getCloneOfOriginalBasicBlock(loopHeader);
    entryBuilder.CreateBr(headerClone);

    /*
     * Add the jump from exit blocks of the loop to the task exit block.
     */
    for (auto exitBB : hottestLoopStructure->getLoopExitBasicBlocks()) {
      auto newExitBB = t->addBasicBlockStub(exitBB);
      t->tagBasicBlockAsLastBlock(newExitBB);
      IRBuilder<> builder(newExitBB);
      builder.CreateBr(t->getExit());
    }

    /*
     * Generate the code in the task to load live-in values from the task
     * environment. While doing that, we replace the declaration of live-in
     * values to be the variables defined in the task by loading from the
     * environment.
     */
    entryBuilder.SetInsertPoint(entryBlockOfTaskBody->getTerminator());
    for (auto envID : envUser->getEnvIDsOfLiveInVars()) {
      auto producer = env->getProducer(envID);
      auto envPointer =
          envUser->createEnvironmentVariablePointer(entryBuilder,
                                                    envID,
                                                    producer->getType());
      auto envLoad = entryBuilder.CreateLoad(envPointer);
      t->addLiveIn(producer, envLoad);
    }

    /*
     * Generate the code in the task to store live-out variables back in the
     * task environment, which is specific of whoever invokes the task.
     */
    auto entryTerminator = entryBlockOfTaskBody->getTerminator();
    IRBuilder<> entryBlockBuilder(entryTerminator);
    auto exitBlockOfTaskBody = t->getExit();
    IRBuilder<> exitBlockOfTaskBodyBuilder(exitBlockOfTaskBody);
    for (auto envID : envUser->getEnvIDsOfLiveOutVars()) {
      auto producer = cast<Instruction>(env->getProducer(envID));
      auto producerClone = t->getCloneOfOriginalInstruction(producer);
      if (!t->doesOriginalLiveOutHaveManyClones(producer)) {
        t->addLiveOut(producer, producerClone);
      }
      auto producerClones = t->getClonesOfOriginalLiveOut(producer);
      auto envType = producer->getType();
      envUser->createEnvironmentVariablePointer(entryBlockBuilder,
                                                envID,
                                                envType);
      auto envPtr = envUser->getEnvPtr(envID);
      for (auto producerClone : producerClones) {
        exitBlockOfTaskBodyBuilder.CreateStore(producerClone, envPtr);
      }
    }

    /*
     * Adjust the data and control flows within the clones included in the task
     * body.
     */
    t->adjustDataAndControlFlowToUseClones();

    /*
     * Now the task is callable by whoever instantiate the task environment
     * before calling it.
     *
     * For this example, let's replace the original loop with a call to the
     * task. First, we need to prepare the inputs we'll pass to the task.
     */
    auto firstBB = hottestLoopFunction->begin();
    auto insertPoint = firstBB->getTerminator()->getPrevNode();
    IRBuilder<> envAllocatorBuilder(&*insertPoint);
    envBuilder->allocateEnvironmentArray(envAllocatorBuilder);
    envBuilder->generateEnvVariables(envAllocatorBuilder);
    for (auto envID : env->getEnvIDsOfLiveInVars()) {
      if (!envBuilder->isIncludedEnvironmentVariable(envID)) {
        continue;
      }
      auto producerOfLiveIn = env->getProducer(envID);
      auto environmentVariable = envBuilder->getEnvironmentVariable(envID);
      auto newStore = envAllocatorBuilder.CreateStore(producerOfLiveIn,
                                                      environmentVariable);
    }

    /*
     * Second, we need to replace the branch of the pre-header of the original
     * loop with a call to the task.
     */
    auto preHeaderBr = loopPreHeader->getTerminator();
    preHeaderBr->eraseFromParent();
    IRBuilder<> callerBuilder(loopPreHeader);
    auto arguments =
        ArrayRef<Value *>({ envBuilder->getEnvironmentArrayVoidPtr() });
    callerBuilder.CreateCall(t->getTaskBody(), arguments);

    /*
     * Third, we need to jump to the only successor of the loop.
     */
    auto loopSuccs = hottestLoopStructure->getLoopExitBasicBlocks();
    auto loopSucc = loopSuccs[0];
    callerBuilder.CreateBr(loopSucc);

    /*
     * Forth, we can now delete the whole original loop.
     */
    for (auto bb : hottestLoopStructure->getBasicBlocks()) {
      bb->eraseFromParent();
    }

    return true;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<NoellePass>();
  }
};

} // namespace

// Next there is code to register your pass to "opt"
char CAT::ID = 0;
static RegisterPass<CAT> X("CAT", "Simple user of the Noelle framework");

// Next there is code to register your pass to "clang"
static CAT *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
                                        [](const PassManagerBuilder &,
                                           legacy::PassManagerBase &PM) {
                                          if (!_PassMaker) {
                                            PM.add(_PassMaker = new CAT());
                                          }
                                        }); // ** for -Ox
static RegisterStandardPasses _RegPass2(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new CAT());
      }
    }); // ** for -O0
