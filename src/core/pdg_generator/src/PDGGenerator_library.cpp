/*
 * Copyright 2016 - 2023  Yian Su, Simone Campanoni
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
#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/TalkDown.hpp"
#include "noelle/core/PDGPrinter.hpp"
#include "noelle/core/PDGGenerator.hpp"
#include "noelle/core/Utils.hpp"

namespace arcana::noelle {

bool PDGGenerator::isTheLibraryFunctionPure(Function *libraryFunction) {
  if (PDGGenerator::externalFuncsHaveNoSideEffectOrHandledBySVF.count(
          libraryFunction->getName())) {
    return true;
  }
  return false;
}

bool PDGGenerator::isTheLibraryFunctionThreadSafe(Function *libraryFunction) {
  if (PDGGenerator::externalThreadSafeFunctions.count(
          libraryFunction->getName())) {
    return true;
  }
  return false;
}

// http://www.cplusplus.com/reference/clibrary/ and
// https://github.com/SVF-tools/SVF/blob/master/lib/Util/ExtAPI.cpp
const StringSet<> PDGGenerator::externalFuncsHaveNoSideEffectOrHandledBySVF{

  // ctype.h
  "isalnum",
  "isalpha",
  "isblank",
  "iscntrl",
  "isdigit",
  "isgraph",
  "islower",
  "isprint",
  "ispunct",
  "isspace",
  "isupper",
  "isxdigit",
  "tolower",
  "toupper",

  // math.h
  "cos",
  "sin",
  "tan",
  "acos",
  "asin",
  "atan",
  "atan2",
  "cosh",
  "sinh",
  "tanh",
  "acosh",
  "asinh",
  "atanh",
  "exp",
  "expf",
  "ldexp",
  "log",
  "logf",
  "log10",
  "exp2",
  "expm1",
  "ilogb",
  "log1p",
  "log2",
  "logb",
  "scalbn",
  "scalbln",
  "pow",
  "sqrt",
  "cbrt",
  "hypot",
  "erf",
  "erfc",
  "tgamma",
  "lgamma",
  "ceil",
  "floor",
  "fmod",
  "trunc",
  "round",
  "lround",
  "llround",
  "nearbyint",
  "remainder",
  "copysign",
  "nextafter",
  "nexttoward",
  "fdim",
  "fmax",
  "fmin",
  "fabs",
  "abs",
  "fma",
  "fpclassify",
  "isfinite",
  "isinf",
  "isnan",
  "isnormal",
  "signbit",
  "isgreater",
  "isgreaterequal",
  "isless",
  "islessequal",
  "islessgreater",
  "isunordered",

  // time.h
  "clock",
  "difftime",

  // wctype.h
  "iswalnum",
  "iswalpha",
  "iswblank",
  "iswcntrl",
  "iswdigit",
  "iswgraph",
  "iswlower",
  "iswprint",
  "iswpunct",
  "iswspace",
  "iswupper",
  "iswxdigit",
  "towlower",
  "towupper",
  "iswctype",
  "towctrans",

  "atoi",
  "atoll",
  "exit",
  "strcmp",
  "strncmp",
  "rand_r",
  "strlen"
};

const StringSet<> PDGGenerator::externalThreadSafeFunctions{

  "malloc",
  "calloc",
  "realloc",
  "free"

};

} // namespace arcana::noelle
