#include <cstdint>
#include <string>

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"

#include "arcana/noelle/core/Lumberjack.hpp"
#include "arcana/noelle/core/Noelle.hpp"
#include "arcana/noelle/core/NoellePass.hpp"

#include "Pass.hpp"

using namespace std;
using namespace llvm;

namespace arcana::noelle {

static cl::list<std::string> ProfileBlackList(
    "profiler-ignore",
    cl::desc(
        "Function prefix to exclude from profiling (e.g., '_ZNSt', '__cxa_', 'llvm.')"),
    cl::value_desc("prefix"),
    cl::ZeroOrMore);

static cl::opt<bool> DisableDefaultBlackList(
    "profiler-no-default-ignore",
    cl::desc("Disable the default C++ runtime function blacklist"),
    cl::init(false));

static cl::opt<bool> DisableAutoCleanup(
    "profiler-no-auto-cleanup",
    cl::desc(
        "Disable automatic cleanup function setup (avoids global constructor issues)"),
    cl::init(false));

// Default blacklist for common C++ runtime and system functions
static const std::vector<std::string> DefaultBlackList = {
  "_ZNSt",                  // std:: namespace functions (mangled)
  "_ZSt",                   // std:: global functions (mangled)
  "_ZN9__gnu_cxx",          // GNU C++ extension functions
  "_ZN4__gnu",              // Additional GNU functions
  "__cxa_",                 // C++ ABI functions (exception handling, etc.)
  "__gxx_",                 // GCC C++ runtime functions
  "_Unwind_",               // Exception unwinding functions
  "llvm.",                  // LLVM intrinsics
  "__stack_chk_",           // Stack protection functions
  "_GLOBAL__sub_",          // Global constructor/destructor functions
  "__clang_call_terminate", // Clang exception handling
  "_ZTI",                   // Type info symbols
  "_ZTV",                   // Virtual table symbols
  "_ZTS",                   // Type string symbols
  "_ZTT",                   // VTT (Virtual Table Table) symbols
  "_ZTH",                   // Thread-local initialization symbols
  "_ZTC",                   // Construction vtable symbols
  "__cxx_global_var_init",  // C++ global variable initialization
  "__dso_handle",           // Dynamic shared object handle
  "_ZTIN",                  // Typeinfo names
  "_ZTSN",                  // Typeinfo strings for namespaces
};

// Helper function to check if a function should be blacklisted
static bool isFunctionBlacklisted(const std::string &functionName) {
  // Always exclude certain problematic functions
  if (functionName.empty() || functionName.find("@") != std::string::npos
      ||                             // Avoid mangled symbols with @
      functionName.find(".") == 0) { // Avoid hidden symbols starting with .
    return true;
  }

  // Check user-specified blacklist
  for (const auto &prefix : ProfileBlackList) {
    if (functionName.find(prefix) == 0) {
      return true;
    }
  }

  // Check default blacklist for C++ runtime functions (unless disabled)
  if (!DisableDefaultBlackList) {
    for (const auto &prefix : DefaultBlackList) {
      if (functionName.find(prefix) == 0) {
        return true;
      }
    }
  }

  return false;
}

// Helper function for debugging - shows which rule matched
static std::string getBlacklistReason(const std::string &functionName) {
  if (functionName.empty())
    return "empty name";
  if (functionName.find("@") != std::string::npos)
    return "contains @";
  if (functionName.find(".") == 0)
    return "starts with .";

  for (const auto &prefix : ProfileBlackList) {
    if (functionName.find(prefix) == 0) {
      return "user blacklist: " + prefix;
    }
  }

  if (!DisableDefaultBlackList) {
    for (const auto &prefix : DefaultBlackList) {
      if (functionName.find(prefix) == 0) {
        return "default blacklist: " + prefix;
      }
    }
  }

  return "not blacklisted";
}

ProfilerPass::ProfilerPass()
  : ModulePass{ ID },
    log(NoelleLumberjack, "Profiler") {}

bool ProfilerPass::doInitialization(Module &M) {
  this->enabled = true;
  return false;
}

void ProfilerPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<NoellePass>();
}

bool ProfilerPass::runOnModule(Module &M) {

  if (!this->enabled) {
    log.debug() << "Profiler is disabled, skipping instrumentation\n";
    return false;
  }

  log.debug() << "Profiler is enabled, starting instrumentation\n";

  // Log blacklist configuration
  if (DisableDefaultBlackList) {
    log.debug() << "Default C++ runtime blacklist is DISABLED\n";
  } else {
    log.debug() << "Default C++ runtime blacklist prefixes:\n";
    auto s = log.indentedSection();
    for (const auto &prefix : DefaultBlackList) {
      log.debug() << "- " << prefix << "\n";
    }
  }

  if (!ProfileBlackList.empty()) {
    log.debug() << "User-specified blacklist prefixes:\n";
    auto s = log.indentedSection();
    for (const auto &prefix : ProfileBlackList) {
      log.debug() << "- " << prefix << "\n";
    }
  } else {
    log.debug() << "No additional user blacklist specified\n";
  }

  auto &context = M.getContext();
  auto *int8PtrTy = Type::getInt8PtrTy(context);
  auto *int64Ty = Type::getInt64Ty(context);
  auto *doubleTy = Type::getDoubleTy(context);
  auto *boolTy = Type::getInt1Ty(context);

  // Declare the stopwatch type matching stopwatch_t from stopwatch.h
  // struct { struct timespec start_time; double elapsed_s; bool running; long
  // laps; const char *name; }
  auto *timespecTy = StructType::create(context, "struct.timespec");
  timespecTy->setBody({ int64Ty, int64Ty }); // tv_sec, tv_nsec

  auto *stopwatchTy = StructType::create(context, "struct.stopwatch_t");
  stopwatchTy->setBody({ timespecTy, doubleTy, boolTy, int64Ty, int8PtrTy });

  log.debug() << "Created stopwatch type definitions\n";

  // Declare the stopwatch functions
  auto *stopwatchFuncTy =
      FunctionType::get(Type::getVoidTy(context),
                        { PointerType::getUnqual(stopwatchTy) },
                        false);
  auto *stopwatchInitFuncTy =
      FunctionType::get(Type::getVoidTy(context),
                        { PointerType::getUnqual(stopwatchTy), int8PtrTy },
                        false);
  auto *stopwatchPrintFuncTy =
      FunctionType::get(Type::getVoidTy(context),
                        { PointerType::getUnqual(stopwatchTy) },
                        false);

  auto stopwatchInit =
      M.getOrInsertFunction("stopwatch_init", stopwatchInitFuncTy);
  auto stopwatchStart =
      M.getOrInsertFunction("stopwatch_start", stopwatchFuncTy);
  auto stopwatchStop = M.getOrInsertFunction("stopwatch_stop", stopwatchFuncTy);
  auto stopwatchPrint =
      M.getOrInsertFunction("stopwatch_print_stats", stopwatchPrintFuncTy);

  log.debug() << "Declared stopwatch runtime functions\n";

  std::vector<GlobalVariable *> stopwatches;
  std::vector<std::string> functionNames;
  int blacklistedCount = 0;

  // For each function, create a global stopwatch and instrument the function
  for (Function &F : M) {
    if (F.isDeclaration())
      continue;

    auto funcName = F.getName().str();

    // Check if function is blacklisted
    if (isFunctionBlacklisted(funcName)) {
      auto reason = getBlacklistReason(funcName);
      log.debug() << "Skipping blacklisted function: " << funcName
                  << " (reason: " << reason << ")\n";
      blacklistedCount++;
      continue;
    }

    log.debug() << "Function passed blacklist check: " << funcName << "\n";

    // Additional safety checks for C++ bitcode
    if (F.empty() || F.getEntryBlock().empty()) {
      log.debug()
          << "Skipping function with no entry block: " << funcName << "\n";
      blacklistedCount++;
      continue;
    }

    // Skip functions with certain attributes that make them unsuitable for
    // instrumentation
    if (F.hasFnAttribute(Attribute::NoInline)
        && F.hasFnAttribute(Attribute::AlwaysInline)) {
      log.debug() << "Skipping function with conflicting inline attributes: "
                  << funcName << "\n";
      blacklistedCount++;
      continue;
    }

    log.debug() << "Instrumenting function: " << funcName << "\n";
    auto s = log.indentedSection();

    // Create a global string for the function name

    // Create the stopwatch global variable (uninitialized)
    auto *stopwatchGV = new GlobalVariable(M,
                                           stopwatchTy,
                                           false,
                                           GlobalValue::InternalLinkage,
                                           Constant::getNullValue(stopwatchTy),
                                           funcName + ".stopwatch");
    stopwatches.push_back(stopwatchGV);
    functionNames.push_back(funcName);

    log.debug() << "Created global stopwatch variable for function\n";

    // Insert stopwatch start at the beginning of the function
    IRBuilder<> builder(&*F.getEntryBlock().getFirstInsertionPt());
    builder.CreateCall(stopwatchStart, { stopwatchGV });

    log.debug() << "Inserted stopwatch start call\n";

    // Insert stopwatch_stop before each return and other exit points
    int returnCount = 0;
    for (auto &BB : F) {
      auto *term = BB.getTerminator();
      if (isa<ReturnInst>(term)) {
        IRBuilder<> retBuilder(term);
        retBuilder.CreateCall(stopwatchStop, { stopwatchGV });
        returnCount++;
      } else if (isa<ResumeInst>(term)) {
        // Handle exception resume - also stop timing
        IRBuilder<> resumeBuilder(term);
        resumeBuilder.CreateCall(stopwatchStop, { stopwatchGV });
        returnCount++;
      }
    }

    log.debug() << "Inserted stopwatch_stop calls before " << returnCount
                << " return statements\n";
  }

  log.debug() << "Instrumented " << stopwatches.size() << " functions\n";
  if (blacklistedCount > 0) {
    log.debug() << "Skipped " << blacklistedCount << " blacklisted functions\n";
  }

  // Initialize stopwatches in main function
  if (!stopwatches.empty()) {
    auto *mainFunc = M.getFunction("main");
    if (mainFunc && !mainFunc->isDeclaration()) {
      // Check if main function is blacklisted (this would be problematic)
      if (isFunctionBlacklisted("main")) {
        log.debug()
            << "Warning: main function is blacklisted but needed for stopwatch initialization\n";
      }

      log.debug() << "Found main function, adding stopwatch initialization\n";

      IRBuilder<> mainBuilder(
          &*mainFunc->getEntryBlock().getFirstInsertionPt());

      // Initialize each stopwatch with its function name
      for (size_t i = 0; i < stopwatches.size(); ++i) {
        auto *stopwatchGV = stopwatches[i];
        auto &funcName = functionNames[i];

        // Create string constant for function name
        auto *funcNameConst =
            ConstantDataArray::getString(context, funcName, true);
        auto *funcNameGV = new GlobalVariable(M,
                                              funcNameConst->getType(),
                                              true,
                                              GlobalValue::PrivateLinkage,
                                              funcNameConst,
                                              funcName + ".name.init");
        funcNameGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

        // Get pointer to the function name string
        auto *zero = ConstantInt::get(Type::getInt32Ty(context), 0);
        Constant *indices[] = { zero, zero };
        auto *namePtr =
            ConstantExpr::getInBoundsGetElementPtr(funcNameConst->getType(),
                                                   funcNameGV,
                                                   indices);

        mainBuilder.CreateCall(stopwatchInit, { stopwatchGV, namePtr });
        log.debug()
            << "Initialized stopwatch for function: " << funcName << "\n";
      }

      log.debug() << "Initialized " << stopwatches.size()
                  << " stopwatches in main function\n";
    } else {
      log.debug()
          << "No main function found or main is declaration only, stopwatches will not be initialized\n";
    }
  }

  // Add a cleanup function that prints all stopwatch results
  if (!stopwatches.empty() && !DisableAutoCleanup) {
    log.debug() << "Creating cleanup function to print timing results\n";

    // Create a global destructor function to print timing results
    auto *cleanupFuncTy =
        FunctionType::get(Type::getVoidTy(context), {}, false);
    auto *cleanupFunc = Function::Create(cleanupFuncTy,
                                         GlobalValue::InternalLinkage,
                                         "print_timing_results",
                                         &M);

    auto *cleanupBB = BasicBlock::Create(context, "entry", cleanupFunc);
    IRBuilder<> cleanupBuilder(cleanupBB);

    // Call stopwatch_print for each stopwatch
    for (auto *sw : stopwatches) {
      cleanupBuilder.CreateCall(stopwatchPrint, { sw });
    }

    cleanupBuilder.CreateRetVoid();

    log.debug() << "Created cleanup function with " << stopwatches.size()
                << " stopwatch print calls\n";

    // Register the cleanup function to run at program exit
    auto *atexitTy =
        FunctionType::get(Type::getInt32Ty(context),
                          { PointerType::getUnqual(cleanupFuncTy) },
                          false);
    auto atexit = M.getOrInsertFunction("atexit", atexitTy);

    // Find or create a global constructor to register the cleanup
    auto *ctorTy = FunctionType::get(Type::getVoidTy(context), {}, false);
    auto *ctorFunc = Function::Create(ctorTy,
                                      GlobalValue::InternalLinkage,
                                      "register_timing_cleanup",
                                      &M);

    auto *ctorBB = BasicBlock::Create(context, "entry", ctorFunc);
    IRBuilder<> ctorBuilder(ctorBB);
    ctorBuilder.CreateCall(atexit, { cleanupFunc });
    ctorBuilder.CreateRetVoid();

    log.debug() << "Created global constructor to register cleanup function\n";

    // Add the constructor to the global constructors list
    auto *ctorStructTy =
        StructType::get(Type::getInt32Ty(context),
                        PointerType::getUnqual(ctorTy),
                        PointerType::getUnqual(Type::getInt8Ty(context)));
    auto *ctorStruct = ConstantStruct::get(
        ctorStructTy,
        { ConstantInt::get(Type::getInt32Ty(context), 65535), // priority
          ctorFunc,
          ConstantPointerNull::get(
              PointerType::getUnqual(Type::getInt8Ty(context))) });

    // Check if llvm.global_ctors already exists
    auto *existingCtors = M.getGlobalVariable("llvm.global_ctors");
    if (existingCtors) {
      log.debug() << "Found existing llvm.global_ctors, appending to it\n";

      // Get the existing array
      auto *existingArray =
          dyn_cast<ConstantArray>(existingCtors->getInitializer());
      if (existingArray) {
        // Create a new array with existing elements plus our new one
        std::vector<Constant *> newElements;
        for (unsigned i = 0; i < existingArray->getNumOperands(); ++i) {
          newElements.push_back(existingArray->getOperand(i));
        }
        newElements.push_back(ctorStruct);

        auto *newCtorArrayTy = ArrayType::get(ctorStructTy, newElements.size());
        auto *newCtorArray = ConstantArray::get(newCtorArrayTy, newElements);

        // Replace the existing global variable
        existingCtors->eraseFromParent();
        new GlobalVariable(M,
                           newCtorArrayTy,
                           false,
                           GlobalValue::AppendingLinkage,
                           newCtorArray,
                           "llvm.global_ctors");
      } else {
        log.debug()
            << "Warning: existing llvm.global_ctors has unexpected format\n";
        // Fallback: create our own array
        auto *ctorArrayTy = ArrayType::get(ctorStructTy, 1);
        auto *ctorArray = ConstantArray::get(ctorArrayTy, { ctorStruct });
        new GlobalVariable(M,
                           ctorArrayTy,
                           false,
                           GlobalValue::AppendingLinkage,
                           ctorArray,
                           "llvm.global_ctors.profiler");
      }
    } else {
      log.debug() << "No existing llvm.global_ctors, creating new one\n";
      // Create new global constructors array
      auto *ctorArrayTy = ArrayType::get(ctorStructTy, 1);
      auto *ctorArray = ConstantArray::get(ctorArrayTy, { ctorStruct });
      new GlobalVariable(M,
                         ctorArrayTy,
                         false,
                         GlobalValue::AppendingLinkage,
                         ctorArray,
                         "llvm.global_ctors");
    }

    log.debug() << "Added constructor to global constructors list\n";
  } else if (DisableAutoCleanup) {
    log.debug() << "Auto cleanup disabled, skipping cleanup setup\n";

    // Instead, create a simple function that users can call manually
    if (!stopwatches.empty()) {
      auto *printAllFuncTy =
          FunctionType::get(Type::getVoidTy(context), {}, false);
      auto *printAllFunc = Function::Create(printAllFuncTy,
                                            GlobalValue::ExternalLinkage,
                                            "profiler_print_all_results",
                                            &M);

      auto *printAllBB = BasicBlock::Create(context, "entry", printAllFunc);
      IRBuilder<> printAllBuilder(printAllBB);

      for (auto *sw : stopwatches) {
        printAllBuilder.CreateCall(stopwatchPrint, { sw });
      }

      printAllBuilder.CreateRetVoid();

      log.debug() << "Created manual profiler_print_all_results() function\n";
    }
  } else {
    log.debug() << "No functions were instrumented, skipping cleanup setup\n";
  }

  // auto &noelle = getAnalysis<NoellePass>().getNoelle();

  return true;
}

char ProfilerPass::ID = 0;
static RegisterPass<ProfilerPass> X("Profiler",
                                    "Noelle profiler",
                                    false,
                                    false);

} // namespace arcana::noelle
