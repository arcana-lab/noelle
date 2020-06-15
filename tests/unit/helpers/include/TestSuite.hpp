/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

#include "Comparators.hpp"

#include <vector>
#include <regex>
#include <string>

using namespace llvm;
using namespace std;

namespace parallelizertests {

  class TestSuite;

  typedef Values (*TestFunction)(ModulePass &, TestSuite &);

  class TestSuite {
   public:

    TestSuite (
      std::string suiteName,
      const char *testNames[],
      TestFunction tests[],
      int numTests,
      std::string actualValuesFileName
    );

    TestSuite(
      std::string suiteName,
      std::vector<std::string> testNames,
      std::vector<TestFunction> tests,
      std::string actualValuesFileName
    );

    ~TestSuite() ;

    void runTests (ModulePass &pass) ;

    bool checkTest (int testId, Values &expectedValues, raw_fd_ostream &File) ;
    std::string addSpacesBetweenDelimiters (std::string delimitedValues) ;
    template <typename T> std::string printToString (T *printable) ;
    template <typename T> std::string printAsOperandToString (T *printable) ;

    std::string valueToString (Value *value) ;
    std::string trimProfilerBitcodeInfo (std::string bitcodeValue) ;

    std::string combineOrderedValues (std::vector<std::string> values) ;
    std::string combineUnorderedValues (std::vector<std::string> values) ;

    static std::string combineValues (std::vector<std::string> values, std::string delimiter) ;

    const std::string orderedValueDelimiter = ";";
    const std::string unorderedValueDelimiter = "|";

   private:

    std::string suiteName;
    std::vector<std::string> testNames;
    std::vector<TestFunction> tests;

    FileComparator *comparator;
  };

  /*
   * Small trait-like static-time check
   */
  struct has_print_impl {
      template <
        typename T,
        typename Print = decltype(std::declval<T&>().print(std::declval<raw_string_ostream &>()))
      > static std::true_type test(int);
      template<typename...> static std::false_type test(...);
  };
  static const int arbitrary_int = 0;
  template<typename T>
  struct has_print : decltype(has_print_impl::test<T>(arbitrary_int)) {};

  template <typename T>
  std::string TestSuite::printToString (T *printable) {
    static_assert(has_print<T>::value, "Must implement a print function");
    std::string str;
    raw_string_ostream valueStream(str);
    printable->print(valueStream);
    valueStream.flush();
    Parser::trim(str);
    return str;
  }

  template <typename T>
  std::string TestSuite::printAsOperandToString (T *printable) {
    std::string str;
    raw_string_ostream valueStream(str);
    printable->printAsOperand(valueStream);
    valueStream.flush();
    Parser::trim(str);
    return str;
  }

}