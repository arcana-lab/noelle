/*
 * Copyright 2023 Xiao Chen
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
#pragma once

#include "noelle/core/Noelle.hpp"
namespace llvm::noelle {

class UserSummary {
public:
  UserSummary(GlobalVariable *globalVar, Noelle &noelle);

  GlobalVariable *globalVar;
  /*
   * All functions that use globalVar.
   */
  unordered_set<Function *> userFunctions;
  /*
   * All users of globalVar classified by function,
   * a user can be an Instruction or an Operator.
   */
  unordered_map<Function *, unordered_set<User *>> users;
  /*
   * All instructions that use globalVar in each function.
   * Instructions may use globalVar directly or indirectly.
   *
   * Direct: `%16 = load i64*, i64** @array, align 8`.
   * Indirect: `store i8* %16, i8** bitcast (i64** @array to i8**), align 8`.
   *
   * The two instructions use global variable @array, so they are both
   * in the `userInsts` set of @array.
   *
   * However, only the loadInst uses @array directly, the storeInst only uses
   * the BitCastOpearator of @array. As a result, the `users` set of @array will
   * contain the loadInst and `i8** bitcast (i64** @array to i8**)`.
   */
  unordered_map<Function *, unordered_set<Instruction *>> userInsts;
};

bool isFixedSizedHeapAllocation(CallBase *heapAllocInst);

/*
 * Get the size of the allocated memory object in bytes.
 */
uint64_t getAllocationSize(Value *allocationSource);

/*
 * Collected all functions that are called directly or indirectly by caller.
 * Caller itself will not be included unless it's called recursively.
 */
unordered_set<Function *> functionsInvokedFrom(Noelle &noelle,
                                               Function *caller);
/*
 * All functions reachable from @main.
 */
unordered_set<Function *> hotFunctions(Noelle &noelle);

} // namespace llvm::noelle
