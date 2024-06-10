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
#include "Repl.hpp"
#include "arcana/noelle/core/Noelle.hpp"
#include "noelle/core/LoopContent.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCCDAG.hpp"

// GNU Readline
#include <readline/history.h>
#include <readline/readline.h>

using namespace llvm;
using namespace std;
using namespace arcana::noelle;

using namespace Repl;

cl::opt<string> HistoryFileName("history",
                                cl::desc("Specify command history file name"),
                                cl::init(""));

class OptRepl : public ModulePass {
public:
  static char ID;
  void getAnalysisUsage(AnalysisUsage &au) const;
  StringRef getPassName() const {
    return "Repl";
  }
  bool runOnModule(Module &M);
  OptRepl() : ModulePass(ID) {}
};

char OptRepl::ID = 0;
static RegisterPass<OptRepl> rp("repl", "Repl");

void OptRepl::getAnalysisUsage(AnalysisUsage &au) const {
  au.addRequired<Noelle>();
  au.setPreservesAll();
}

// a simple autocompletion generator
char *completion_generator(const char *text, int state) {
  // This function is called with state=0 the first time; subsequent calls are
  // with a nonzero state. state=0 can be used to perform one-time
  // initialization for this completion session.
  static std::vector<std::string> matches;
  static size_t match_index = 0;

  if (state == 0) {
    // During initialization, compute the actual matches for 'text' and keep
    // them in a static vector.
    matches.clear();
    match_index = 0;

    // Collect a vector of matches: vocabulary words that begin with text.
    std::string textstr = std::string(text);
    for (auto word : ReplVocab) {
      if (word.size() >= textstr.size()
          && word.compare(0, textstr.size(), textstr) == 0) {
        matches.push_back(word);
      }
    }
  }

  if (match_index >= matches.size()) {
    // We return nullptr to notify the caller no more matches are available.
    return nullptr;
  } else {
    // Return a malloc'd char* for the match. The caller frees it.
    return strdup(matches[match_index++].c_str());
  }
}

char **completer(const char *text, int start, int end) {
  // Don't do filename completion even if our generator finds no matches.
  rl_attempted_completion_over = 1;

  // Note: returning nullptr here will make readline use the default filename
  // completer.
  return rl_completion_matches(text, completion_generator);
}

void ReplDriver::funcsFn() {
  outs() << "List of functions:\n";

  for (auto &F : this->M) {
    if (F.isDeclaration()) {
      continue;
    }
    outs() << F.getName() << ":\n";
    for (auto attr : F.getAttributes()) {
      outs() << "\t" << attr.getAsString() << "\n";
    }
  }
}

void ReplDriver::loopsFn() {
  outs() << "List of hot loops:\n";

  for (auto &[loopId, loop] : loopIdMap) {
    auto ls = loop->getLoopStructure();
    outs() << loopId << ": " << ls->getHeader()->getParent()->getName()
           << "::" << ls->getHeader()->getName() << '\n';
    outs() << "  Coverage in terms of total instructions = "
           << profiles->getDynamicTotalInstructionCoverage(ls) * 100 << "\n";
  }
}

void ReplDriver::selectFn() {
  selectedLoopId = parser.getActionId();
  if (selectedLoopId == -1) {
    outs() << "No number specified\n";
    return;
  }

  if (loopIdMap.find(selectedLoopId) == loopIdMap.end()) {
    outs() << "Loop " << selectedLoopId << " does not exist\n";
    return;
  }

  auto loop = loopIdMap[selectedLoopId];
  auto ls = loop->getLoopStructure();
  outs() << "Selecting loop " << selectedLoopId << ": ";
  outs() << ls->getHeader()->getParent()->getName()
         << "::" << ls->getHeader()->getName() << '\n';
  selectedLoop = loop;

  selectedPDG = std::make_unique<PDG>(*loop->getLoopDG());
  selectedSCCDAG = std::make_unique<SCCDAG>(selectedPDG.get());

  createInstIdMap(M, selectedPDG.get());
  createInstIdLookupMap();
}

void ReplDriver::helpFn() {
  string action = parser.getStringAfterAction();
  if (ReplActions.find(action) != ReplActions.end()) {
    outs() << HelpText.at(ReplActions.at(action)) << "\n";
  } else {
    for (auto &[action, explaination] : HelpText) {
      outs() << explaination << "\n";
    }
  }
}

void ReplDriver::quitFn() {
  state.is_terminate = true;
}

void ReplDriver::dumpFn() {
  auto ls = selectedLoop->getLoopStructure();
  ls->print(outs());
  outs() << "Number of instructions: "
         << selectedPDG->getNumberOfInstructionsIncluded() << "\n";
  outs() << "Number of dependences: "
         << selectedPDG->getNumberOfDependencesBetweenInstructions() << "\n";
  outs() << "Number of SCCs: " << selectedSCCDAG->numNodes();

  outs() << "\n";

  if (parser.isVerbose()) {
    for (auto block : ls->getBasicBlocks()) {
      outs() << *block;
    }
  }
  outs() << "\n";
}

void ReplDriver::instsFn() {
  int instId = parser.getActionId();
  if (instId != -1) {
    if (instIdMap->find(instId) == instIdMap->end()) {
      outs() << "instId " << instId << " not found!\n";
    } else {
      outs() << instId << "\t" << *instIdMap->at(instId)->getT() << "\n";
    }
  } else {
    for (auto &[instId, node] : *instIdMap) {
      outs() << instId << "\t" << *node->getT() << "\n";
    }
  }
}

void ReplDriver::depsFn() {
  int fromId = parser.getFromId();
  int toId = parser.getToId();
  if (fromId != -1) {
    if (instIdMap->find(fromId) == instIdMap->end()) {
      outs() << "From InstId " << fromId << " not found\n";
      return;
    }
  }

  if (toId != -1) {
    if (instIdMap->find(toId) == instIdMap->end()) {
      outs() << "To InstId " << toId << " not found\n";
      return;
    }
  }

  depIdMap = std::make_unique<DepIdMap_t>();
  unsigned id = 0;
  if (fromId == -1 && toId == -1) { // both not specified
    for (auto &edge : selectedPDG->getEdges()) {
      dumpEdge(id, edge);
      depIdMap->insert(make_pair(id++, edge));
    }
  } else if (fromId != -1 && toId != -1) { // both specified
    auto fromNode = instIdMap->at(fromId);
    auto toNode = instIdMap->at(toId);
    for (auto &edge : fromNode->getSrcEdges()) {
      if (edge->getDstNode() == toNode) {
        dumpEdge(id, edge);
        depIdMap->insert(make_pair(id++, edge));
      }
    }
  } else if (fromId != -1) { // from is specified
    auto node = instIdMap->at(fromId);
    for (auto &edge : node->getSrcEdges()) {
      dumpEdge(id, edge);
      depIdMap->insert(make_pair(id++, edge));
    }
  } else if (toId != -1) { // to is specified
    auto node = instIdMap->at(toId);
    for (auto &edge : node->getDstEdges()) {
      dumpEdge(id, edge);
      depIdMap->insert(make_pair(id++, edge));
    }
  }

  createDepIdLookupMap(*depIdMap);
  selectedPDG->setDepLookupMap(depIdLookupMap);
  llvm::noelle::DGPrinter::writeClusteredGraph<PDG, Value>("currentPDG.dot",
                                                           selectedPDG.get());
}

void ReplDriver::removeFn() {
  int depId = parser.getActionId();
  if (depId == -1) {
    outs() << "No number specified\n";
    return;
  }

  if (depIdMap->find(depId) == depIdMap->end()) {
    outs() << "DepId" << depId << " not found\n";
    return;
  }

  auto dep = depIdMap->at(depId);
  selectedPDG->removeEdge(dep);
  // update SCCDAG
  selectedSCCDAG = std::make_unique<SCCDAG>(selectedPDG.get());
}

void ReplDriver::removeAllFn() {
  int instId = parser.getActionId();
  if (instId == -1) {
    outs() << "No number specified\n";
    return;
  }

  if (instIdMap->find(instId) == instIdMap->end()) {
    outs() << "InstId" << instId << " not found\n";
    return;
  }

  auto node = instIdMap->at(instId);
  list<llvm::noelle::DGEdge<Value, Value> *> edgesToRemove;
  for (auto &edge : node->getSrcEdges()) {
    edgesToRemove.push_back(edge);
  }

  for (auto &edge : node->getDstEdges()) {
    edgesToRemove.push_back(edge);
  }

  for (auto edge : edgesToRemove) {
    selectedPDG->removeEdge(edge);
  }
  // update SCCDAG
  selectedSCCDAG = std::make_unique<SCCDAG>(selectedPDG.get());
}

void ReplDriver::parallelizeFn() {
  outs() << "Unimplemented\n";
}

void ReplDriver::modrefFn() {
  outs() << "Unimplemented\n";
}

void ReplDriver::saveFn() {
  string fileName = parser.getStringAfterAction();
  if (fileName == "") {
    fileName = "repl_command_history.log";
  }
  // remove the current command from readline history
  remove_history(history_length - 1);
  write_history(fileName.c_str());
  outs()
      << "command history (excluding \"save\" command) has been written into "
      << fileName << "\n";
}

string ReplDriver::prompt() {
  stringstream ss;
  ss << "(noelle-repl";
  if (selectedLoopId != -1)
    ss << " loop " << selectedLoopId;
  ss << ") ";
  return ss.str();
}

bool OptRepl::runOnModule(Module &M) {
  bool modified = false;

  auto &noelle = getAnalysis<Noelle>();

  ReplDriver driver(noelle, M);
  driver.createLoopMap();

  rl_attempted_completion_function = completer;

  // execute command history file if specified
  string historyFileName = HistoryFileName;
  if (historyFileName != "") {
    read_history(historyFileName.c_str());
    // DISCUSSION: the last command won't get executed if using 'i <
    // history_length'
    for (int i = history_base; i <= history_length; i++) {
      char *buf = history_get(i)->line;
      string query = (const char *)(buf);
      driver.run(query);
    }
    clear_history();
  }

  // the main repl while loop
  while (true) {
    if (driver.hasTerminated()) {
      break;
    }

    char *buf = readline(driver.prompt().c_str());
    if (!buf) {
      outs() << "Quit\n";
      break;
    }
    string query = (const char *)(buf);
    if (query.size() > 0) {
      add_history(buf);
      free(buf); // free the buf readline created
    }
    driver.run(query);
  }

  return modified;
}
