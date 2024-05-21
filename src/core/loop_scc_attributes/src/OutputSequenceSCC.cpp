/*
 * Copyright 2024 Sophia Boksenbaum, Simone Campanoni
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

#include <limits>
#include "noelle/core/OutputSequenceSCC.hpp"

namespace arcana::noelle {

OutputSequenceSCC::OutputSequenceSCC(
    SCC *s,
    LoopStructure *loop,
    const std::set<DGEdge<Value, Value> *> &loopCarriedDependences)
  : LoopCarriedSCC{ SCCKind::OUTPUT_SEQUENCE,
                    s,
                    loop,
                    loopCarriedDependences,
                    false } {
  return;
}

std::optional<int> OutputSequenceSCC::getMaxNumberOfPrintedBytes(CallInst *I) {
  if (auto fstring = OutputSequenceSCC::printConstantFstring(I)) {
    return OutputSequenceSCC::fstringMaxLength(fstring.value());
  }
  return {};
}

std::optional<std::string> OutputSequenceSCC::printConstantFstring(
    CallInst *I) {
  auto calledName = I->getCalledFunction()->getName();
  if (auto gep = dyn_cast<GetElementPtrInst>(
          I->getOperand(calledName == "printf" ? 0 : 1))) {
    if (auto gv = dyn_cast<GlobalVariable>(gep->getOperand(0))) {
      if (auto cda = dyn_cast<ConstantDataArray>(gv->getInitializer())) {
        return std::string(cda->getAsString());
      }
    }
  }
  return {};
}

// parses only %[width][.precision](f|d|i)
std::optional<int> OutputSequenceSCC::fstringMaxLength(std::string str) {
  // number of digits in smallest negative + 3 for -0.
  constexpr int doubleMaxLen = 3 + std::numeric_limits<double>::digits10
                               + -std::numeric_limits<double>::min_exponent10;
  static const int intMaxLen = std::to_string(INT_MIN).length();

  // % starts a placeholder, . starts precision
  enum PARSE_STATE { NONE, START_PLACEHOLDER, PRECISION };

  PARSE_STATE state = NONE;
  // missing width/precision specifiers are treated as 0.
  // this is fine because we always take the max with the
  // no-specifiers max length.
  int width = 0;
  int precision = 0;

  int totalLength = 0;

  for (const char &ch : str) {
    if (state == NONE) {
      if (ch == '%') {
        state = START_PLACEHOLDER;
        width = 0;
        precision = 0;
      } else {
        totalLength++;
      }
    } else {
      switch (ch) {
        case 'f': {
          // the width field left-pads with spaces, but does nothing
          // if the printed number is longer.
          // the precision field determines the number of digits after
          // the decimal point (which can be much greater than the actual
          // precision of the number). max_exponent10 is the largest n
          // such that 10^n is representable, which is one less than the max
          // printed length. the other 1 is for the decimal point.
          totalLength += std::max(
              width,
              2 + std::numeric_limits<double>::max_exponent10 + precision);
          state = NONE;
        } break;
        case 'd':
        case 'i': {
          // width pads with spaces and precision pads with 0s, but precision
          // doesn't consider negatives
          totalLength += std::max(intMaxLen, std::max(width, 1 + precision));
          state = NONE;
        } break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
          char atNumber = ch - '0';
          switch (state) {
            // this must be part of the width field
            case START_PLACEHOLDER:
              width = (10 * width) + atNumber;
              break;
            case PRECISION:
              precision = (10 * precision) + atNumber;
              break;
          }
        } break;
        case '.':
          state = PRECISION;
          break;
        // give up on anything other than f/d/i, a digit, or .
        default:
          return {};
      }
    }
  }
  return totalLength;
}

bool OutputSequenceSCC::classof(const GenericSCC *s) {
  return (s->getKind() >= GenericSCC::SCCKind::OUTPUT_SEQUENCE)
         && (s->getKind() <= GenericSCC::SCCKind::LAST_OUTPUT_SEQUENCE);
}
} // namespace arcana::noelle
