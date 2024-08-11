/*
 * Copyright 2019 - 2021  Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_BASIC_UTILITIES_UTILS_H_
#define NOELLE_SRC_CORE_BASIC_UTILITIES_UTILS_H_

#include "arcana/noelle/core/SystemHeaders.hpp"

namespace arcana::noelle {

class Utils {
public:
  static bool isActualCode(Instruction *inst);

  static bool isAllocator(CallBase *callInst);

  static bool isReallocator(CallBase *callInst);

  static bool isDeallocator(CallBase *callInst);

  static Value *getAllocatedObject(CallBase *call);

  static Value *getFreedObject(CallBase *call);

  template <class T>
  static std::vector<T *> sort(const std::unordered_set<T *> &s);

  template <class T>
  static std::vector<T *> sort(const std::set<T *> &s);

  static Value *injectPrint(Value *toPrint,
                            std::string format,
                            IRBuilder<> &builder);

  static Value *injectPrint(std::vector<Value *> &toPrint,
                            std::string format,
                            IRBuilder<> &builder);

  static Value *injectPrint(std::string toPrint, IRBuilder<> &builder);
};

template <class T>
std::vector<T *> Utils::sort(const std::unordered_set<T *> &s) {
  std::vector<T *> v;

  /*
   * Sort the instructions.
   */
  for (auto i : s) {
    v.push_back(i);
  }
  auto compareInstructions = [](T *i, T *j) { return i < j; };
  std::sort(v.begin(), v.end(), compareInstructions);

  return v;
}

template <class T>
std::vector<T *> Utils::sort(const std::set<T *> &s) {
  std::vector<T *> v;

  /*
   * Sort the instructions.
   */
  for (auto i : s) {
    v.push_back(i);
  }
  auto compareInstructions = [](T *i, T *j) { return i < j; };
  std::sort(v.begin(), v.end(), compareInstructions);

  return v;
}

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_BASIC_UTILITIES_UTILS_H_
