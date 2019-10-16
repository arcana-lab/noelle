/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "TestSuite.hpp"

using namespace parallelizertests;

TestSuite::TestSuite (
  std::string suite,
  const char *names[],
  TestFunction fns[],
  int numTests,
  std::string actualValuesFileName
) : suiteName{suite} {
  for (auto i = 0; i < numTests; ++i) {
    this->testNames.push_back(std::string(names[i]));
    this->tests.push_back(fns[i]);
  }
  this->comparator = new FileComparator(actualValuesFileName, unorderedValueDelimiter, orderedValueDelimiter);
}

TestSuite::TestSuite (
  std::string suite,
  std::vector<std::string> names,
  std::vector<TestFunction> testFns,
  std::string actualValuesFileName
) : suiteName{suite}, testNames{names}, tests{testFns} {
  this->comparator = new FileComparator(actualValuesFileName);
}

void TestSuite::runTests (ModulePass &pass) {
  Values groups(testNames.begin(), testNames.end());
  std::pair<Values, Values>
  mismatchGroups = comparator->nonIntersectingGroups(groups);
  if (mismatchGroups.second.size() != 0) {
    for (auto group : mismatchGroups.second) {
      errs() << suiteName << ": Test not found: " << group << "\n";
    }
    return;
  }

  for (auto group : mismatchGroups.first) {
    errs() << suiteName << ": Not Testing: " << group << "\n";
  }

  for (auto testId = 0; testId < tests.size(); ++testId) {
    std::string testName(testNames[testId]);
    if (mismatchGroups.first.find(testName) != mismatchGroups.first.end()) continue;
    Values expected = tests[testId](pass);
    checkTest(testId, expected);
  }
}

void TestSuite::checkTest (int testId, Values &expectedValues) {
  std::pair<Values, Values>
  mismatchValues = comparator->nonIntersectingOfGroup(testNames[testId], expectedValues);

  errs() << "\n";
  bool testPassed = true;
  for (auto v : mismatchValues.first) {
    testPassed = false;
    errs() << suiteName << ": Expected: " << v << " not found.\n";
  }
  for (auto v : mismatchValues.second) {
    testPassed = false;
    errs() << suiteName << ": Not expected: " << v << " yet found\n";
  }

  if (testPassed) errs() << suiteName << ": Passed: " << testNames[testId] << "\n";
  else errs() << suiteName << ": Failed: " << testNames[testId] << "\n";
}