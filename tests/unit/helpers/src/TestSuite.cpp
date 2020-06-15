/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "TestSuite.hpp"

namespace parallelizertests {

TestSuite::TestSuite (
  std::string suite,
  const char *names[],
  TestFunction fns[],
  int numTests,
  std::string expectedValuesFileName
) : suiteName{suite} {
  for (auto i = 0; i < numTests; ++i) {
    this->testNames.push_back(std::string(names[i]));
    this->tests.push_back(fns[i]);
  }
  this->comparator = new FileComparator(expectedValuesFileName, unorderedValueDelimiter, orderedValueDelimiter);

}

TestSuite::TestSuite (
  std::string suite,
  std::vector<std::string> names,
  std::vector<TestFunction> testFns,
  std::string expectedValuesFileName
) : suiteName{suite}, testNames{names}, tests{testFns} {
  this->comparator = new FileComparator(expectedValuesFileName);

}

TestSuite::~TestSuite() {}

void TestSuite::runTests (ModulePass &pass) {
  std::error_code EC;
  raw_fd_ostream File("test_output.txt", EC);
  if (EC) {
    errs() << "Could not open output file!\n";
    return ;
  }

  Values groups(testNames.begin(), testNames.end());
  std::pair<Values, Values>
  mismatchGroups = comparator->nonIntersectingGroups(groups);
  if (mismatchGroups.second.size() != 0) {
    for (auto group : mismatchGroups.second) {
      File << suiteName << ": Test not found: " << group << "\n";
    }
    return;
  }

  for (auto group : mismatchGroups.first) {
    File << suiteName << ": Not Testing: " << group << "\n";
  }

  int numSuccess = 0;
  int numSkips = 0;
  for (auto testId = 0; testId < tests.size(); ++testId) {
    std::string testName(testNames[testId]);
    if (mismatchGroups.first.find(testName) != mismatchGroups.first.end()) {
      numSkips++;
      continue;
    }

    Values actual = tests[testId](pass, *this);
    numSuccess += checkTest(testId, actual, File) ? 1 : 0;
  }

  File << suiteName << " Summary: Successes: " << numSuccess
    << " Skips: " << numSkips
    << " Failures: " << (tests.size() - numSuccess - numSkips) << "\n";

  File.close();
}

bool TestSuite::checkTest (int testId, Values &actualValues, raw_fd_ostream &File) {
  std::pair<Values, Values>
  mismatchValues = comparator->nonIntersectingOfGroup(testNames[testId], actualValues);

  bool testPassed = true;
  for (auto v : mismatchValues.second) {
    testPassed = false;
    File << suiteName << ": Expected    : " << addSpacesBetweenDelimiters(v) << " not found.\n";
  }
  for (auto v : mismatchValues.first) {
    testPassed = false;
    File << suiteName << ": Not expected: " << addSpacesBetweenDelimiters(v) << " yet found\n";
  }

  if (testPassed) File << suiteName << ": Passed: " << testNames[testId] << "\n";
  else File << suiteName << ": Failed: " << testNames[testId] << "\n";

  return testPassed;
}

std::string TestSuite::addSpacesBetweenDelimiters (std::string delimitedValues) {
  std::regex delimiterRegex("\\" + orderedValueDelimiter + "|" + "\\" + unorderedValueDelimiter);
  std::string captured = "$&";
  std::string spacedDelimiterRegex(" $& ");

  std::string spacedOutput;
  raw_string_ostream valueStream(spacedOutput);
  valueStream << std::regex_replace(delimitedValues, delimiterRegex, spacedDelimiterRegex);
  valueStream.flush();
  return spacedOutput;
}

std::string TestSuite::valueToString (Value *value) {
  return trimProfilerBitcodeInfo(printToString(value));
}

std::string TestSuite::trimProfilerBitcodeInfo (std::string bitcodeValue) {
  auto it = bitcodeValue.find(", !prof");
  if (it != std::string::npos) {
    bitcodeValue.erase(bitcodeValue.begin() + it, bitcodeValue.end());
  }
  return bitcodeValue;
}

std::string TestSuite::combineOrderedValues (std::vector<std::string> values) {
  return TestSuite::combineValues(values, orderedValueDelimiter);
}

std::string TestSuite::combineUnorderedValues (std::vector<std::string> values) {
  return TestSuite::combineValues(values, unorderedValueDelimiter);
}

std::string TestSuite::combineValues (std::vector<std::string> values, std::string delimiter) {
  std::string allValues = values[0];
  for (int i = 1; i < values.size(); ++i) {
    allValues += delimiter + values[i];
  }

  return allValues;
}

}