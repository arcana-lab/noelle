/*
 * Copyright 2016 - 2022  Yian Su, Ziyang Xu, Simone Campanoni
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
#pragma once

#include "noelle/core/SystemHeaders.hpp"
using std::string, std::map, std::vector;

// All possible actions
enum ReplAction {
  Help = 0,
  Loops,
  Select,
  Quit,
  Dump,
  Insts,
  Deps,
  Remove,
  RemoveAll,
  Parallelize,
  Modref,
  Save,
  Unknown = -1
};

// the string command to the action Enum
const map<string, ReplAction> ReplActions = {
  { "help", ReplAction::Help },
  { "h", ReplAction::Help },
  { "loops", ReplAction::Loops },
  { "ls", ReplAction::Loops },
  { "select", ReplAction::Select },
  { "s", ReplAction::Select },
  { "quit", ReplAction::Quit },
  { "q", ReplAction::Quit },
  { "dump", ReplAction::Dump },
  { "d", ReplAction::Dump },
  { "insts", ReplAction::Insts },
  { "is", ReplAction::Insts },
  { "deps", ReplAction::Deps },
  { "ds", ReplAction::Deps },
  { "remove", ReplAction::Remove },
  { "r", ReplAction::Remove },
  { "removeall", ReplAction::RemoveAll },
  { "removeAll", ReplAction::RemoveAll },
  { "ra", ReplAction::RemoveAll },
  { "parallelize", ReplAction::Parallelize },
  { "p", ReplAction::Parallelize },
  { "modref", ReplAction::Modref },
  { "save", ReplAction::Save },
};

// a helper to get the vocabulary of Repl, to help the auto completion
const vector<string> ReplVocab = [](map<string, ReplAction> map) {
  vector<string> v;
  for (auto &[s, a] : map) {
    v.push_back(s);
  }
  v.push_back("from");
  v.push_back("to");
  return v;
}(ReplActions);

// the action to help text
const map<ReplAction, string> HelpText = {
  { Help, "help/h (command): \tprint help message (for certain command)" },
  { Loops, "loops/ls: \tprint all loops with loop id" },
  { Select, "select/s \t$loop_id: select a loop to work with" },
  { Dump,
    "dump (-v):\t dump the loop information (verbose: dump the loop instructions)" },
  { Insts, "insts/is: \tshow instructions with instruction id" },
  { Deps,
    "deps/ds (from $inst_id_from) (to $inst_id_to): \tshow dependences with dependence id (from or to certain instructions)" },
  { Remove, "remove/r $dep_id: \tremove a certain dependence from the loop" },
  { RemoveAll,
    "removeAll/ra $inst_id: \tremove all dependences from and to a instruction from the loop" },
  { Parallelize,
    "paralelize/p: \tparallelize the selected loop with current dependences" },
  { Modref,
    "modref/mr $inst_id1, $inst_id2: \tquery the modref between two instructions" },
  { Save, "save repl commands to an output file" },
  { Quit, "quit/q: quit the repl" },
};

class ReplParser {
private:
  string originString;

  // check whether a string is only digits
  static bool isNumber(const string &s) {
    for (char const &c : s) {
      if (std::isdigit(c) == 0)
        return false;
    }
    return true;
  }

  // get the next number after a keyword
  int getQueryNumber(string query) {
    auto pos = originString.find(query);
    if (pos == string::npos)
      return -1;

    pos += query.size()
           + 1; // move to the start of the number (+1 is the space between)
    if (pos >= originString.size())
      return -1;
    // get the number
    string number = originString.substr(pos, originString.find(" ", pos) - pos);
    if (!isNumber(number)) {
      return -1;
    } else {
      return stoi(number);
    }
  }

public:
  ReplParser(string str) : originString(str) {}

  // get the action for the command
  ReplAction getAction() {
    string firstWord;
    firstWord =
        originString.substr(0, originString.find(" ")); // first space or end

    if (ReplActions.find(firstWord) != ReplActions.end()) {
      return ReplActions.at(firstWord);
    } else {
      return ReplAction::Unknown;
    }
  }

  // get the next string after action
  string getStringAfterAction() {
    auto secondWordStartPos = originString.find(" ");
    if (secondWordStartPos == string::npos) {
      return "";
    }

    secondWordStartPos += 1;

    auto secondWordLen =
        originString.find(" ", secondWordStartPos) - secondWordStartPos;
    string secondStr = originString.substr(secondWordStartPos, secondWordLen);
    return secondStr;
  }

  // the number after action
  int getActionId() {
    string firstWord;
    firstWord =
        originString.substr(0, originString.find(" ")); // first space or end

    return getQueryNumber(firstWord);
  }

  // the number after from
  int getFromId() {
    string query = "from";
    return getQueryNumber(query);
  }

  // the number after to
  int getToId() {
    string query = "to";
    return getQueryNumber(query);
  }

  bool isVerbose() {
    if (originString.find("-v") != string::npos) {
      return true;
    } else {
      return false;
    }
  };
};
