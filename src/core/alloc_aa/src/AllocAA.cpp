/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/AllocAA.hpp"

namespace llvm::noelle {

/*
 * Options of the custom allocation function based alias analysis pass.
 */
static cl::opt<int> Verbose(
    "alloc-aa-verbose",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Verbose output (0: disabled, 1: minimal, 2: maximal"));

bool AllocAA::doInitialization(Module &M) {
  this->readOnlyFunctionNames = {};
  this->allocatorFunctionNames = { "malloc", "calloc" };
  this->memorylessFunctionNames = {
    "sqrt",
    "sqrtf",
    "ceil",
    "floor",
    "log",
    "log10",
    "pow",
    "exp",
    "cos",
    "acos",
    "sin",
    "tanh",
    "atoll",
    "atoi",
    "atol"

    // C++
    ,
    "_ZSt4fmaxIiiEN9__gnu_cxx11__promote_2IT_T0_NS0_9__promoteIS2_Xsr3std12__is_integerIS2_EE7__valueEE6__typeENS4_IS3_Xsr3std12__is_integerIS3_EE7__valueEE6__typeEE6__typeES2_S3_"
  };

  this->verbose = static_cast<AllocAAVerbosity>(Verbose.getValue());

  return false;
}

bool AllocAA::runOnModule(Module &M) {
  if (this->verbose != AllocAAVerbosity::Disabled) {
    errs() << "AllocAA at \"runOnModule\"\n";
  }

  auto &callGraph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  collectCGUnderFunctionMain(M, callGraph);
  collectAllocations(M, callGraph);
  collectPrimitiveArrayValues(M);
  collectMemorylessFunctions(M);

  return false;
}

void AllocAA::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<CallGraphWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.setPreservesAll();
  return;
}

std::pair<Value *, GetElementPtrInst *> AllocAA::getPrimitiveArrayAccess(
    Value *V) {
  auto memOp = getMemoryPointerOperand(V);
  if (!memOp)
    return std::make_pair(nullptr, nullptr);

  /*
   * The value V is a memory instruction directly on an array
   */
  auto directAccessArray = getPrimitiveArray(memOp);
  if (directAccessArray) {
    // directAccessArray->print(errs() << "Found direct access array: "); errs()
    // << "\n";
  }
  if (directAccessArray)
    return std::make_pair(directAccessArray, nullptr);

  auto empty = std::make_pair(nullptr, nullptr);
  if (auto gep = dyn_cast<GetElementPtrInst>(memOp)) {

    /*
     * The value V is a memory instruction on a GEP of either a
     * local array or a load of a global array
     */
    auto gepMemOp = gep->getPointerOperand();
    auto localArray = getLocalPrimitiveArray(gepMemOp);
    if (localArray) {
      // localArray->print(errs() << "Found GEP access local array: "); errs()
      // << "\n";
    }
    if (localArray)
      return std::make_pair(localArray, gep);

    auto loadMemOp = getMemoryPointerOperand(gepMemOp);
    auto globalArray =
        loadMemOp ? getGlobalValuePrimitiveArray(loadMemOp) : nullptr;
    if (globalArray) {
      // globalArray->print(errs() << "Found GEP access global array: "); errs()
      // << "\n";
    }
    if (globalArray)
      return std::make_pair(globalArray, gep);
  }
  return empty;
}

/*
 * Check that all non-constant indices of GEP are those of monotonic induction
 * variables
 * TODO: Replace with more strict check that all uses of the GEP adhere to base
 * type of pointer
 */
bool AllocAA::areGEPIndicesConstantOrIV(GetElementPtrInst *gep) {
  Function *gepFunc = gep->getFunction();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>(*gepFunc).getSE();

  for (auto &indexV : gep->indices()) {
    if (isa<ConstantInt>(indexV))
      continue;

    // Assumption? : All polynomial add recursive expressions are induction
    // variables
    auto scev = SE.getSCEV(indexV);
    if (scev->getSCEVType() != scAddRecExpr)
      return false;
  }
  return true;
}

bool AllocAA::areIdenticalGEPAccessesInSameLoop(GetElementPtrInst *gep1,
                                                GetElementPtrInst *gep2) {
  if (gep1 == gep2)
    return true;

  if (gep1->getFunction() != gep2->getFunction())
    return false;
  auto &LI =
      getAnalysis<LoopInfoWrapperPass>(*gep1->getFunction()).getLoopInfo();
  if (LI.getLoopFor(gep1->getParent()) != LI.getLoopFor(gep2->getParent()))
    return false;

  auto gepOp1 = gep1->getPointerOperand();
  auto gepOp2 = gep2->getPointerOperand();
  if (gepOp1 != gepOp2) {
    Value *accessed = nullptr;
    if (auto load = dyn_cast<LoadInst>(gepOp1)) {
      accessed = load->getPointerOperand();
    } else
      return false;
    if (auto load = dyn_cast<LoadInst>(gepOp2)) {
      if (accessed != load->getPointerOperand())
        return false;
    } else
      return false;
  }

  auto indexCount = 0;
  for (auto &indexV1 : gep1->indices()) {
    auto &indexV2 = *(gep2->idx_begin() + indexCount++);
    if (indexV1 != indexV2)
      return false;
  }

  return true;
}

bool AllocAA::isReadOnly(StringRef functionName) {
  return readOnlyFunctionNames.find(functionName)
         != readOnlyFunctionNames.end();
}

bool AllocAA::isMemoryless(StringRef functionName) {
  return memorylessFunctionNames.find(functionName)
         != memorylessFunctionNames.end();
}

void AllocAA::collectCGUnderFunctionMain(Module &M, CallGraph &callGraph) {

  /*
   * Fetch main
   */
  auto main = M.getFunction("main");
  assert(main != nullptr);

  /*
   * Traverse the functions
   */
  std::queue<Function *> funcToTraverse;
  std::set<Function *> reached;
  funcToTraverse.push(main);
  reached.insert(main);
  while (!funcToTraverse.empty()) {
    auto func = funcToTraverse.front();
    funcToTraverse.pop();

    auto funcCGNode = callGraph[func];
    for (auto &callRecord :
         make_range(funcCGNode->begin(), funcCGNode->end())) {
      auto F = callRecord.second->getFunction();
      if (!F || F->empty())
        continue;

      if (reached.find(F) != reached.end())
        continue;
      reached.insert(F);
      funcToTraverse.push(F);
    }
  }

  CGUnderMain.clear();
  CGUnderMain.insert(reached.begin(), reached.end());
}

void AllocAA::collectAllocations(Module &M, CallGraph &callGraph) {
  std::set<Function *> allocatorFns;
  for (auto allocName : allocatorFunctionNames) {
    auto F = M.getFunction(allocName);
    if (!F)
      continue;
    allocatorFns.insert(F);
  }
  collectFunctionCallsTo(callGraph, allocatorFns, this->allocatorCalls);
}

void AllocAA::collectFunctionCallsTo(CallGraph &callGraph,
                                     std::set<Function *> &called,
                                     std::set<CallInst *> &calls) {
  for (auto caller : CGUnderMain) {
    auto funcCGNode = callGraph[caller];
    for (auto &callRecord :
         make_range(funcCGNode->begin(), funcCGNode->end())) {
      auto F = callRecord.second->getFunction();
      if (called.find(F) == called.end())
        continue;
      if (auto call = dyn_cast<CallInst>(&*callRecord.first)) {
        calls.insert(call);
      }
    }
  }
}

bool AllocAA::collectUserInstructions(
    Value *V,
    std::set<Instruction *> &userInstructions) {
  for (auto user : V->users()) {
    Instruction *I = nullptr;
    if (isa<Instruction>(user)) {
      I = (Instruction *)user;
    } else if (isa<BitCastOperator>(user) || isa<ZExtOperator>(user)) {
      if (user->hasOneUse()) {
        auto operUser = *user->user_begin();
        if (isa<Instruction>(operUser)) {
          I = (Instruction *)operUser;
        }
      }
    }

    if (!I)
      return false;
    userInstructions.insert(I);
  }
  return true;
}

void AllocAA::collectPrimitiveArrayValues(Module &M) {

  /*
   * Check global values used under the CG of function "main"
   */
  for (auto &GV : M.globals()) {
    if (GV.hasExternalLinkage())
      continue;
    if (GV.getNumUses() == 0)
      continue;

    std::set<Instruction *> scopedUsers;
    if (!collectUserInstructions(&GV, scopedUsers))
      continue;

    bool relevantToMain = false;
    for (auto I : scopedUsers) {
      relevantToMain |= CGUnderMain.find(I->getFunction()) != CGUnderMain.end();
      if (relevantToMain)
        break;
    }
    if (!relevantToMain)
      continue;

    if (isPrimitiveArrayPointer(&GV, scopedUsers))
      primitiveArrayGlobals.insert(&GV);
  }

  /*
   * Check values where contiguous memory allocators are used
   */
  for (auto call : allocatorCalls) {
    std::set<Instruction *> allUsers;
    if (!collectUserInstructions(call, allUsers))
      continue;
    if (isPrimitiveArray(call, allUsers))
      primitiveArrayLocals.insert(call);
  }
}

bool AllocAA::isPrimitiveArrayPointer(
    Value *V,
    std::set<Instruction *> &userInstructions) {
  bool isPrimitive = true;
  for (auto I : userInstructions) {
    if (auto store = dyn_cast<StoreInst>(I)) {
      // Confirm the store is of a contiguously allocated array unique to this
      // value
      if (auto storedCall = dyn_cast<CallInst>(store->getValueOperand())) {
        auto callF = storedCall->getCalledFunction();
        // Conservatively return false for indirect calls
        if (!callF) {
          return false;
        }
        if (allocatorFunctionNames.find(callF->getName())
            != allocatorFunctionNames.end()) {
          if (storedCall->hasOneUse())
            continue;
        }
      }
    }

    if (auto load = dyn_cast<LoadInst>(I)) {
      // Confirm all uses of the GV load obey those of a primitive array
      std::set<Instruction *> allUsers;
      if (collectUserInstructions(load, allUsers)
          && isPrimitiveArray(load, allUsers)) {
        continue;
      }
    }

    if (verbose >= AllocAAVerbosity::Maximal) {
      I->print(errs() << "AllocAA:  GV related instruction not understood: ");
      errs() << "\n";
    }
    isPrimitive = false;
    break;
  }

  if (verbose >= AllocAAVerbosity::Minimal) {
    errs() << "AllocAA:  GV value is a primitive integer array: " << isPrimitive
           << ", ";
    V->print(errs());
    errs() << "\n";
  }
  return isPrimitive;
}

bool AllocAA::isPrimitiveArray(Value *V,
                               std::set<Instruction *> &userInstructions) {
  auto isPrimitive = true;
  for (auto I : userInstructions) {
    if (auto cast = dyn_cast<CastInst>(I)) {
      std::set<Instruction *> castUsers;
      if (collectUserInstructions(cast, castUsers)
          && isPrimitiveArray(cast, castUsers))
        continue;
    }
    if (auto GEPUser = dyn_cast<GetElementPtrInst>(I)) {
      if (doesValueNotEscape({ GEPUser }, GEPUser))
        continue;
    }
    if (auto callUser = dyn_cast<CallInst>(I)) {
      auto calleeFn = callUser->getCalledFunction();
      if (calleeFn != nullptr) {
        auto fnName = calleeFn->getName();
        if (readOnlyFunctionNames.find(fnName) != readOnlyFunctionNames.end())
          continue;
      }
    }

    if (verbose >= AllocAAVerbosity::Maximal) {
      I->print(errs() << "AllocAA:  related instruction not understood: ");
      errs() << "\n";
    }
    isPrimitive = false;
    break;
  }

  if (verbose >= AllocAAVerbosity::Minimal) {
    errs() << "AllocAA:  value is a primitive integer array: " << isPrimitive
           << ", ";
    V->print(errs());
    errs() << "\n";
  }
  return isPrimitive;
}

bool AllocAA::doesValueNotEscape(std::set<Instruction *> checked,
                                 Instruction *I) {
  User *unkUser = nullptr;
  for (auto user : I->users()) {
    if (!isa<Instruction>(user)) {
      unkUser = user;
      break;
    }
    auto userI = cast<Instruction>(user);
    if (checked.find(userI) != checked.end())
      continue;
    checked.insert(userI);

    /*
     * The termination is either local to the function, or the return value
     * is not escaped (only an integer type is returned)
     */
    Instruction *userInst;
    if (true && ((userInst = dyn_cast<Instruction>(user)) != nullptr)
        && userInst->isTerminator()) {
      if (isa<BranchInst>(user) || isa<SwitchInst>(user))
        continue;
      if (isa<ReturnInst>(user)) {

        /*
         * NOTE: Technically, a program could treat the returned integer as
         * a pointer, but since at no point along the uses of the original
         * value are pointer based instructions permitted, no intentional
         * pointer value can be returned here
         */
        auto returnV = cast<ReturnInst>(user)->getReturnValue();
        if (isa<IntegerType>(returnV->getType()))
          continue;
      }
      unkUser = user;
      break;
    }

    /*
     * The user stores a non-escaped value into the memory location
     */
    if (auto store = dyn_cast<StoreInst>(user)) {
      auto stored = store->getValueOperand();
      auto storedDoesNotEscape = false;
      if (isa<IntegerType>(stored->getType())) {
        if (auto storedI = dyn_cast<Instruction>(stored)) {
          if (doesValueNotEscape(checked, storedI))
            storedDoesNotEscape = true;
        }
        if (auto storedC = dyn_cast<ConstantData>(stored))
          storedDoesNotEscape = true;
      }

      auto storage = store->getPointerOperand();
      auto storageDoesNotEscape = storage == (Value *)I;
      if (!storageDoesNotEscape) {
        if (auto storageI = dyn_cast<Instruction>(storage)) {
          if (doesValueNotEscape(checked, storageI))
            storageDoesNotEscape = true;
        }
      }

      if (storedDoesNotEscape && storageDoesNotEscape)
        continue;
      unkUser = user;
      break;
    }

    /*
     * The user is a non-escaped integer expression
     */
    if (isa<IntegerType>(userI->getType())) {
      if (doesValueNotEscape(checked, userI))
        continue;
    }
    unkUser = user;
    break;
  }

  if (unkUser) {
    if (verbose >= AllocAAVerbosity::Maximal) {
      unkUser->print(errs() << "AllocAA:  GV related user not understood: ");
      errs() << "\n";
      unkUser->getType()->print(errs() << "AllocAA:  \tWith type");
      errs() << "\n";
    }
    return false;
  }
  return true;
}

void AllocAA::collectMemorylessFunctions(Module &M) {
  for (auto F : this->CGUnderMain) {

    auto isMemoryless = true;
    for (auto &B : *F) {
      for (auto &I : B) {
        if (isa<LoadInst>(I) || isa<StoreInst>(I) || isa<CallInst>(I)) {
          isMemoryless = false;
        }

        for (auto &op : I.operands()) {
          if (isa<GlobalValue>(op.get())) {
            isMemoryless = false;
            break;
          }
        }

        if (!isMemoryless)
          break;
      }

      if (!isMemoryless)
        break;
    }

    // TODO(angelo): Trigger a recheck of functions using this function
    // in case they are then found to be memoryless
    if (isMemoryless) {
      memorylessFunctionNames.insert(F->getName());
      if (verbose >= AllocAAVerbosity::Minimal) {
        errs()
            << "AllocAA:  Memoryless function found: " << F->getName() << "\n";
      }
    }
  }
}

Value *AllocAA::getPrimitiveArray(Value *V) {
  auto localArray = getLocalPrimitiveArray(V);
  return localArray ? localArray : getGlobalValuePrimitiveArray(V);
}

Value *AllocAA::getLocalPrimitiveArray(Value *V) {
  auto targetV = V;
  if (auto cast = dyn_cast<CastInst>(V))
    targetV = cast->getOperand(0);
  if (auto I = dyn_cast<Instruction>(targetV)) {
    if (primitiveArrayLocals.find(I) != primitiveArrayLocals.end()) {
      return I;
    }
  }
  return nullptr;
}

Value *AllocAA::getGlobalValuePrimitiveArray(Value *V) {
  auto targetV = V;
  if (auto cast = dyn_cast<CastInst>(V))
    targetV = cast->getOperand(0);
  if (auto GV = dyn_cast<GlobalValue>(targetV)) {
    if (primitiveArrayGlobals.find(GV) != primitiveArrayGlobals.end()) {
      return GV;
    }
  }
  return nullptr;
}

Value *AllocAA::getMemoryPointerOperand(Value *V) {
  if (auto load = dyn_cast<LoadInst>(V)) {
    return load->getPointerOperand();
  }
  if (auto store = dyn_cast<StoreInst>(V)) {
    return store->getPointerOperand();
  }
  return nullptr;
}

bool AllocAA::canPointToTheSameObject(Value *p1, Value *p2) {

  /*
   * Exploit arguments.
   */
  if (!this->canPointToTheSameObject_ArgumentAttributes(p1, p2)) {
    return false;
  }

  /*
   * Exploit library knowledge.
   */
  if (!this->canPointToTheSameObject_Globals(p1, p2)) {
    return false;
  }

  return true;
}

Value *AllocAA::getBasePointer(Value *p) {
  assert(p != nullptr);

  if (auto gep = dyn_cast<GetElementPtrInst>(p)) {
    return gep->getPointerOperand();
  }

  return p;
}

bool AllocAA::canPointToTheSameObject_Globals(Value *p1, Value *p2) {

  /*
   * Fetch the base pointers.
   */
  auto b1 = this->getBasePointer(p1);
  auto b2 = this->getBasePointer(p2);

  /*
   * Check if the base pointers are incompatible.
   */
  if (isa<GlobalValue>(b1) && isa<AllocaInst>(b2)) {
    return false;
  }
  if (isa<GlobalValue>(b2) && isa<AllocaInst>(b1)) {
    return false;
  }
  if (isa<GlobalValue>(b1) && isa<GlobalValue>(b2) && (b1 != b2)) {
    return false;
  }
  if (isa<AllocaInst>(b1) && isa<AllocaInst>(b2) && (b1 != b2)) {
    return false;
  }

  return true;
}

bool AllocAA::canPointToTheSameObject_ArgumentAttributes(Value *p1, Value *p2) {

  /*
   * Fetch the load instruction
   */
  auto loadInst = dyn_cast<LoadInst>(p1);
  if (!loadInst) {
    loadInst = dyn_cast<LoadInst>(p2);
  }
  if (loadInst == nullptr) {
    return true;
  }

  /*
   * Fetch the store instruction
   */
  auto storeInst = dyn_cast<StoreInst>(p1);
  if (!storeInst) {
    storeInst = dyn_cast<StoreInst>(p2);
  }
  if (storeInst == nullptr) {
    return true;
  }

  /*
   * Fetch the pointer to the object accessed by the load instruction.
   */
  auto loadPtr = loadInst->getPointerOperand();
  if (auto gep = dyn_cast<GetElementPtrInst>(loadPtr)) {
    loadPtr = gep->getPointerOperand();
  }
  assert(loadPtr != nullptr);

  /*
   * Check if the object is read-only
   */
  auto obj1 = dyn_cast<Argument>(loadPtr);
  if (obj1 == nullptr) {
    return true;
  }
  if (obj1->onlyReadsMemory()) {
    return false;
  }

  return true;
}

// Next there is code to register your pass to "opt"
char AllocAA::ID = 0;
static RegisterPass<AllocAA> X("AllocAA", "Dependence Graph modifier");

// Next there is code to register your pass to "clang"
static AllocAA *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
                                        [](const PassManagerBuilder &,
                                           legacy::PassManagerBase &PM) {
                                          if (!_PassMaker) {
                                            PM.add(_PassMaker = new AllocAA());
                                          }
                                        }); // ** for -Ox
static RegisterStandardPasses _RegPass2(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new AllocAA());
      }
    }); // ** for -O0

} // namespace llvm::noelle
