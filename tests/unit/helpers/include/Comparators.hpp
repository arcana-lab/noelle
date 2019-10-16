/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/Support/raw_ostream.h"

#include <fstream>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>
#include <string>

using namespace llvm;
using namespace std;

namespace parallelizertests {
  using Values = std::set<std::string>;

  class Comparator {
   public:
    std::pair<Values, Values> nonIntersecting (Values &v, Values &w);
  };

  class FileComparator : public Comparator {
   public:
    FileComparator (std::string filename, std::string unorderedDelimiter = "|", std::string orderedDelimiter = ";");
  
    std::pair<Values, Values> nonIntersectingGroups (Values &groups);
    std::pair<Values, Values> nonIntersectingOfGroup (std::string group, Values &values);

    const std::unordered_map<std::string, std::set<std::string>> &getGroupValues ();

   private:

    std::string processDelimitedRow (std::string value);

    std::string filename;
    std::string unorderedDelimiter, orderedDelimiter;
    std::unordered_map<std::string, Values> groupValues;
  };

  class Parser {
   public:
    static void ltrim (std::string &s);
    static void rtrim (std::string &s);
    static void trim (std::string &s);
    static std::vector<std::string> split (std::string s, std::string delimiter);
  };
}
