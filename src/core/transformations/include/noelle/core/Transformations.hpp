/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_TRANSFORMATIONS_H_
#define NOELLE_SRC_CORE_TRANSFORMATIONS_H_
#pragma once

namespace arcana::noelle {

/*
 * Transformations included in NOELLE
 */
enum Transformation {
  DOALL_ID,
  DSWP_ID,
  HELIX_ID,
  INLINER_ID,
  LOOP_UNROLLER_ID,
  LOOP_DISTRIBUTION_ID,
  LOOP_INVARIANT_CODE_MOTION_ID,
  LOOP_WHILIFIER_ID,
  SCEV_SIMPLIFICATION_ID,
  DEVIRTUALIZER_ID,

  First = DOALL_ID,
  Last = DEVIRTUALIZER_ID
};

enum LoopDependenceInfoOptimization {
  MEMORY_CLONING_ID,
  THREAD_SAFE_LIBRARY_ID
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_TRANSFORMATIONS_H_
