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

#include "noelle/core/Noelle.hpp"
#include "noelle/core/SystemHeaders.hpp"
#include <iostream>
#include <utility>
using std::string, std::map, std::vector;

// define the base virtual class of ReplAction

namespace Repl {

enum ReplAction {
  Help = 0,
  Functions,
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
  { "funcs", ReplAction::Functions },
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
  v.emplace_back("from");
  v.emplace_back("to");
  return v;
}(ReplActions);

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
  ReplParser(string str) : originString(std::move(str)) {}

  // TODO: check is the string is well-formed
  void parse(string str) {
    originString = str;
  }

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

class ReplDriver {
protected:
  using ActionFunc = std::function<void()>;
  const map<ReplAction, string> HelpText = {
    { Help, "help/h (command): \tprint help message (for certain command)" },
    { Loops, "loops/ls: \tprint all loops with loop id" },
    { Functions, "funcs: \tprint all functions with body and their attributes" },
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
    { Quit, "quit/q: quit the repl" }
  };

  map<ReplAction, ActionFunc> actionFuncs;

  Module &M;
  Noelle &noelle;

  Hot *profiles;
  std::vector<llvm::noelle::LoopDependenceInfo *> *loops;

  ReplParser parser;
  /// The Driver State
  struct State {
    bool is_terminate = false;
  };

  State state;
  // FIXME: put in the state
  int selectedLoopId = -1;
  // the selected information
  llvm::Function *selectedFunction; // TODO: not used yet
  LoopDependenceInfo *selectedLoop;
  unique_ptr<PDG> selectedPDG;
  unique_ptr<SCCDAG> selectedSCCDAG;

  using InstIdMap_t = map<unsigned, DGNode<Value> *>;
  using InstIdReverseMap_t = map<DGNode<Value> *, unsigned>;
  using DepIdMap_t = map<unsigned, DGEdge<Value> *>;
  using DepIdReverseMap_t = map<DGEdge<Value> *, unsigned>;

  // store the loopID
  map<unsigned, LoopDependenceInfo *> loopIdMap;

  unique_ptr<InstIdMap_t> instIdMap;
  unique_ptr<InstIdReverseMap_t> instIdLookupMap;
  unique_ptr<DepIdMap_t> depIdMap;
  shared_ptr<DepIdReverseMap_t> depIdLookupMap;

  void createInstIdLookupMap() {
    auto lookupMap = std::make_unique<InstIdReverseMap_t>();
    for (auto &[instId, node] : *instIdMap) {
      lookupMap->insert(make_pair(node, instId));
    }

    instIdLookupMap = std::move(lookupMap);
  }

  static bool hasPDGAsMetadata(Module &M) {
    if (auto n = M.getNamedMetadata("noelle.module.pdg")) {
      if (auto m = dyn_cast<MDNode>(n->getOperand(0))) {
        if (cast<MDString>(m->getOperand(0))->getString() == "true") {
          return true;
        }
      }
    }

    return false;
  }

  static unsigned getNoelleInstId(DGNode<Value> *instNode) {
    if (auto inst = dyn_cast<Instruction>(instNode->getT())) {
      if (auto m = inst->getMetadata("noelle.pdg.inst.id")) {
        if (auto cam = dyn_cast<ConstantAsMetadata>(m->getOperand(0))) {
          if (auto constant = dyn_cast<ConstantInt>(cam->getValue())) {
            return (unsigned)constant->getSExtValue();
          }
        }
      }
    }
    assert(false && "found an instruction without instruction id\n");
  }

  void createInstIdMap(Module &M, PDG *pdg) {
    auto instIdMap = std::make_unique<InstIdMap_t>();
    // the pdg is embedded as metadata in the module
    // use noelle.pdg.inst.id as id for each instruction
    if (hasPDGAsMetadata(M)) {
      for (auto &instNode : pdg->getNodes()) {
        unsigned noelleInstId = getNoelleInstId(instNode);
        assert((instIdMap->find(noelleInstId) == instIdMap->end())
               && "Found noelle instructions that share the same id\n");
        instIdMap->insert(make_pair(noelleInstId, instNode));
      }
    } else {
      unsigned instId = 0;
      for (auto &instNode : pdg->getNodes()) {
        instIdMap->insert(make_pair(instId, instNode));
        instId++;
      }
    }

    this->instIdMap = std::move(instIdMap);
  }

  void createDepIdLookupMap(DepIdMap_t m) {
    auto lookupMap = std::make_shared<DepIdReverseMap_t>();
    for (auto &[instId, node] : m) {
      lookupMap->insert(make_pair(node, instId));
    }

    depIdLookupMap = std::move(lookupMap);
  }

  // helper function for dumping edge
  void dumpEdge(unsigned depId, DGEdge<Value> *edge) {
    auto idA = instIdLookupMap->at(edge->getOutgoingNode());
    auto idB = instIdLookupMap->at(edge->getIncomingNode());
    outs() << depId << "\t" << idA << "->" << idB << ":\t" << edge->toString()
           << (edge->isLoopCarriedDependence() ? "(LC)" : "(LL)") << "\n";
  };

public:
  ReplDriver(llvm::noelle::Noelle &noelle, Module &m)
    : noelle(noelle),
      M(m),
      parser("") {
    actionFuncs = { { Help, [this] { this->helpFn(); } },
                    { Functions, [this] { this->funcsFn(); } },
                    { Loops, [this] { this->loopsFn(); } },
                    { Select, [this] { this->selectFn(); } },
                    { Dump, [this] { this->dumpFn(); } },
                    { Insts, [this] { this->instsFn(); } },
                    { Deps, [this] { this->depsFn(); } },
                    { Remove, [this] { this->removeFn(); } },
                    { RemoveAll, [this] { this->removeAllFn(); } },
                    { Parallelize, [this] { this->parallelizeFn(); } },
                    { Modref, [this] { this->modrefFn(); } },
                    { Save, [this] { this->saveFn(); } },
                    { Quit, [this] { this->quitFn(); } } };
  }

  // get all loops and assign them id based on hotness decrementally
  virtual void createLoopMap() {
    profiles = noelle.getProfiles();
    loops = noelle.getLoops();
    noelle.sortByHotness(*loops);
    unsigned loopId = 0;
    for (auto loop : *loops) {
      loopIdMap[loopId++] = loop;
      continue;
    }
  }

  virtual void helpFn();
  virtual void funcsFn();
  virtual void loopsFn();
  virtual void selectFn();
  virtual void dumpFn();
  virtual void instsFn();
  virtual void depsFn();
  virtual void removeFn();
  virtual void removeAllFn();
  virtual void parallelizeFn();
  virtual void modrefFn();
  virtual void saveFn();
  virtual void quitFn();

  virtual ~ReplDriver() = default;

  virtual string prompt();

  /// @brief Run one action
  /// @param action The action to run
  /// @return true if the action is quit
  /// @return false if the action is not quit
  virtual void run(string &query) {
    parser.parse(query);
    auto action = parser.getAction();
    if (action == ReplAction::Unknown) {
      outs() << "Unknown command!\n";
      return;
    }
    auto actionFunc = actionFuncs[action];
    actionFunc();
  }

  bool hasTerminated() {
    return state.is_terminate;
  }
};

} // namespace Repl
