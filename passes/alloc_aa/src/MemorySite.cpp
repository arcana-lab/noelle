/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "../include/MemorySite.hpp"

using namespace llvm;
using namespace std;

AllocAAResult llvm::MemorySiteInfo::doesAlias (Value *V1, Value *V2) {

  /*
   * One or both values is not understood
   */
  auto ref1 = referenceSites.find(V1);
  if (ref1 == referenceSites.end()) return AllocAAResult::May;
  auto ref2 = referenceSites.find(V2);
  if (ref2 == referenceSites.end()) return AllocAAResult::May;

  /*
   * Both values escape
   */
  auto site1 = ref1->second;
  auto site2 = ref2->second;
  if (site1 == site2) return AllocAAResult::Must;
  if (site1->escapingValues.size() > 0 && site2->escapingValues.size() > 0) return AllocAAResult::May;

  /*
   * The values are not known to reference each other,
   * and at least one site is fully understood. That
   * ensures the two values do not alias
   */
  return AllocAAResult::No;
}
