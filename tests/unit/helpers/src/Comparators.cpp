/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Comparators.hpp"

namespace parallelizertests {

std::pair<Values, Values> Comparator::nonIntersecting (Values &v, Values &w) {
  Values notV, notW;
  for (auto x : v) {
    if (w.find(x) == w.end()) {
      notW.insert(x);
    }
  }
  for (auto y : w) {
    if (v.find(y) == v.end()) {
      notV.insert(y);
    }
  }

  return make_pair(notV, notW);
}

FileComparator::FileComparator (std::string filename, std::string unordered, std::string ordered)
  : unorderedDelimiter{unordered}, orderedDelimiter{ordered} {
  ifstream file;
  file.open(filename);
  if (!file.is_open()) {
    errs() << "Could not open file: " << filename << "\n";
    throw;
  }

  const std::set<std::string> lineContinuations { orderedDelimiter, unorderedDelimiter };

  std::string line;
  std::string group = "";
  std::vector<std::string> lineSplits{};
  while (getline(file, line)) {
    Parser::trim(line);
    if (line == "") {
      group = "";
      continue;
    }

    if (line[0] == '#') {
      continue;
    }

    if (group == "") {
      group = line;
      groupValues[group].clear();
    } else {
      lineSplits.push_back(line);
      std::string lastCharacter(1, line[line.length() - 1]);
      if (lineContinuations.find(lastCharacter) == lineContinuations.end()) {
        std::string fullLine;
        for (auto l : lineSplits) fullLine += l;
        lineSplits.clear();
        groupValues[group].insert(processDelimitedRow(fullLine));
      }
    }
  }

  file.close();
}

std::string FileComparator::processDelimitedRow (std::string value) {
  Parser::trim(value);

  /*
   * Determine if the whole value represents an ordered/unordered set of tokens
   */
  std::vector<std::string> unorderedTokens{}, orderedTokens{};
  trySplitOrderedAndUnordered(value, orderedTokens, unorderedTokens);
  bool isUnordered = unorderedTokens.size() > 1;
  bool isOrdered = orderedTokens.size() > 1;

  /*
   * Trim each token, and if unordered, sort lexicographically
   */
  if (!isUnordered && !isOrdered) return value;
  auto &tokens = isUnordered ? unorderedTokens : orderedTokens;
  for (auto &token : tokens) Parser::trim(token);
  if (isUnordered) std::sort(tokens.begin(), tokens.end());
  
  /*
   * Recombine the tokens
   */
  std::string result = tokens[0];
  for (int i = 1; i < tokens.size(); ++i) result += orderedDelimiter + tokens[i];
  return result;
}

std::pair<Values, Values> FileComparator::nonIntersectingGroups (std::set<std::string> &groupNames) {
  Values selfGroups{};
  for (auto pair : groupValues) selfGroups.insert(pair.first);
  return nonIntersecting(selfGroups, groupNames);
}

std::pair<Values, Values> FileComparator::nonIntersectingOfGroup (std::string group, Values &values) {
  Values processedValues{};
  for (auto value : values) processedValues.insert(processDelimitedRow(value));
  return nonIntersecting(this->groupValues[group], processedValues);
}

const std::unordered_map<std::string, std::set<std::string>> &FileComparator::getGroupValues () {
  return this->groupValues;
}

std::vector<std::string> FileComparator::split (std::string value) {
  std::vector<std::string> unorderedTokens;
  std::vector<std::string> orderedTokens;
  trySplitOrderedAndUnordered(value, orderedTokens, unorderedTokens);
  return orderedTokens.size() > 1 ? orderedTokens : unorderedTokens;
}

void FileComparator::trySplitOrderedAndUnordered (
  std::string value,
  std::vector<std::string> &ordered,
  std::vector<std::string> &unordered
) {
  std::vector<std::string> unorderedTokens = Parser::split(value, unorderedDelimiter);
  std::vector<std::string> orderedTokens = Parser::split(value, orderedDelimiter);
  bool isUnordered = unorderedTokens.size() > 1;
  bool isOrdered = orderedTokens.size() > 1;
  assert(!(isUnordered && isOrdered)
    && "Error: tests cannot mix unordered and ordered expected values");

  ordered.clear();
  ordered.insert(ordered.begin(), orderedTokens.begin(), orderedTokens.end());
  unordered.clear();
  unordered.insert(unordered.begin(), unorderedTokens.begin(), unorderedTokens.end());
}

void Parser::ltrim (std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
      return !std::isspace(ch);
  }));
}

void Parser::rtrim (std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
      return !std::isspace(ch);
  }).base(), s.end());
}

void Parser::trim (std::string &s) {
  ltrim(s);
  rtrim(s);
}

std::vector<std::string> Parser::split (std::string s, std::string delimiter) {
  size_t prev_pos = 0, pos = 0;
  std::vector<std::string> tokens{};
  while (prev_pos < s.length() && (pos = s.find(delimiter, prev_pos)) != std::string::npos) {
    tokens.push_back(s.substr(prev_pos, pos - prev_pos));
    prev_pos = pos + 1;
  }
  if (prev_pos != s.length()) tokens.push_back(s.substr(prev_pos, s.length() - prev_pos));
  return tokens;
}

}