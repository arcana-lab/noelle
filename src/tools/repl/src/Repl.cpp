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
#include "noelle/core/Noelle.hpp"
#include "noelle/core/LoopDependenceInfo.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/SCCDAG.hpp"

// GNU Readline
#include <readline/history.h>
#include <readline/readline.h>

using namespace llvm;
using namespace std;
using namespace llvm::noelle;

cl::opt<string> HistoryFileName("history",
                                cl::desc("Specify command history file name"),
                                cl::init(""));

class OptRepl : public ModulePass {
public:
  static char ID;
  void getAnalysisUsage(AnalysisUsage &au) const;
  StringRef getPassName() const {
    return "remed-selector";
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

typedef map<unsigned, DGNode<Value> *> InstIdMap_t;
typedef map<DGNode<Value> *, unsigned> InstIdReverseMap_t;
typedef map<unsigned, DGEdge<Value> *> DepIdMap_t;
typedef map<DGEdge<Value> *, uint32_t> DepIdReverseMap_t;

bool hasPDGAsMetadata(Module &M) {
  if (auto n = M.getNamedMetadata("noelle.module.pdg")) {
    if (auto m = dyn_cast<MDNode>(n->getOperand(0))) {
      if (cast<MDString>(m->getOperand(0))->getString() == "true") {
        return true;
      }
    }
  }

  return false;
}

unsigned getNoelleInstId(DGNode<Value> *instNode) {
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
  exit(1);
}

// helper function to generate
static unique_ptr<InstIdMap_t> createInstIdMap(Module &M, PDG *pdg) {
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

  return instIdMap;
}

static unique_ptr<InstIdReverseMap_t> createInstIdLookupMap(InstIdMap_t m) {
  auto lookupMap = std::make_unique<InstIdReverseMap_t>();
  for (auto &[instId, node] : m) {
    lookupMap->insert(make_pair(node, instId));
  }

  return lookupMap;
}

static shared_ptr<DepIdReverseMap_t> createDepIdLookupMap(DepIdMap_t m) {
  auto lookupMap = std::make_shared<DepIdReverseMap_t>();
  for (auto &[instId, node] : m) {
    lookupMap->insert(make_pair(node, instId));
  }

  return lookupMap;
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

bool OptRepl::runOnModule(Module &M) {
  bool modified = false;

  auto &noelle = getAnalysis<Noelle>();
  auto profiles = noelle.getProfiles();
  auto loops = noelle.getLoops();

  // store the loopID
  map<unsigned, LoopDependenceInfo *> loopIdMap;

  // the selected information
  llvm::Function *selectedFunction; // TODO: not used yet
  LoopDependenceInfo *selectedLoop;
  unique_ptr<PDG> selectedPDG;
  unique_ptr<SCCDAG> selectedSCCDAG;

  unique_ptr<InstIdMap_t> instIdMap;
  unique_ptr<InstIdReverseMap_t> instIdLookupMap;
  unique_ptr<DepIdMap_t> depIdMap;
  shared_ptr<DepIdReverseMap_t> depIdLookupMap;

  // get all loops and assign them id based on hotness decrementally
  {
    noelle.sortByHotness(*loops);
    unsigned loopId = 0;
    for (auto i = 0; i < loops->size(); i++) {
      auto *loop = loops->at(i);
      loopIdMap[loopId++] = loop;
      continue;
    }
  }

  rl_attempted_completion_function = completer;
  int selectLoopId = -1;
  bool quit = false;
  auto mainLoop = [&](string &query) {
    // check if it's quit or unknown
    ReplParser parser(query);
    if (parser.getAction() == ReplAction::Quit) {
      quit = true;
      return;
    }

    if (parser.getAction() == ReplAction::Unknown) {
      outs() << "Unknown command!\n";
      return;
    }

    // print all loops
    auto loopsFn = [&loopIdMap, &profiles]() {
      outs() << "List of hot loops:\n";

      for (auto &[loopId, loop] : loopIdMap) {
        auto ls = loop->getLoopStructure();
        outs() << loopId << ": " << ls->getHeader()->getParent()->getName()
               << "::" << ls->getHeader()->getName() << '\n';
        outs()
            << "  Coverage in terms of total instructions = "
            << profiles->getDynamicTotalInstructionCoverage(ls) * 100 << "\n";
      }
    };

    // select one loop
    auto selectFn = [&M,
                     &selectLoopId,
                     &loopIdMap,
                     &parser,
                     &selectedLoop,
                     &selectedPDG,
                     &selectedSCCDAG,
                     &instIdMap,
                     &instIdLookupMap]() {
      selectLoopId = parser.getActionId();
      if (selectLoopId == -1) {
        outs() << "No number specified\n";
        return;
      }

      if (loopIdMap.find(selectLoopId) == loopIdMap.end()) {
        outs() << "Loop " << selectLoopId << " does not exist\n";
        return;
      }

      auto loop = loopIdMap[selectLoopId];
      auto ls = loop->getLoopStructure();
      outs() << "Selecting loop " << selectLoopId << ": ";
      outs() << ls->getHeader()->getParent()->getName()
             << "::" << ls->getHeader()->getName() << '\n';
      selectedLoop = loop;

      selectedPDG = std::make_unique<PDG>(*loop->getLoopDG());
      selectedSCCDAG = std::make_unique<SCCDAG>(selectedPDG.get());

      instIdMap = createInstIdMap(M, selectedPDG.get());
      instIdLookupMap = createInstIdLookupMap(*instIdMap);
    };

    // show help
    auto helpFn = [&parser]() {
      string action = parser.getStringAfterAction();
      if (ReplActions.find(action) != ReplActions.end()) {
        outs() << HelpText.at(ReplActions.at(action)) << "\n";
      } else {
        for (auto &[action, explaination] : HelpText) {
          outs() << explaination << "\n";
        }
      }
    };

    // early checks for several actions that do not need the loop set
    if (parser.getAction() == ReplAction::Loops) {
      loopsFn();
      return;
    }

    if (parser.getAction() == ReplAction::Select) {
      selectFn();
      return;
    }

    if (parser.getAction() == ReplAction::Help) {
      helpFn();
      return;
    }

    // after this assume the loop has been selected
    if (!selectedLoop) {
      outs() << "No loops selected\n";
      return;
    }

    // dump information about the loop
    auto dumpFn = [&parser, &selectedLoop, &selectedPDG, &selectedSCCDAG]() {
      auto ls = selectedLoop->getLoopStructure();
      ls->print(outs());
      outs() << "Number of instructions: "
             << selectedPDG->getNumberOfInstructionsIncluded() << "\n";
      outs()
          << "Number of dependences: "
          << selectedPDG->getNumberOfDependencesBetweenInstructions() << "\n";
      outs() << "Number of SCCs: " << selectedSCCDAG->numNodes();

      outs() << "\n";

      if (parser.isVerbose()) {
        for (auto block : ls->getBasicBlocks()) {
          outs() << *block;
        }
      }
      outs() << "\n";
    };

    // show instructions with id
    auto InstsFn = [&instIdMap, &parser]() {
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
    };

    // helper function for dumping edge
    auto dumpEdge = [&instIdLookupMap](unsigned depId, DGEdge<Value> *edge) {
      auto idA = instIdLookupMap->at(edge->getOutgoingNode());
      auto idB = instIdLookupMap->at(edge->getIncomingNode());
      outs() << depId << "\t" << idA << "->" << idB << ":\t" << edge->toString()
             << (edge->isLoopCarriedDependence() ? "(LC)" : "(LL)") << "\n";
    };

    // show all deps with id; also generate a currentPDG.dot file, the edge
    // number is annotated on the PDG
    auto depsFn = [&instIdLookupMap,
                   &parser,
                   &depIdMap,
                   &depIdLookupMap,
                   &selectedPDG,
                   &dumpEdge,
                   &instIdMap]() {
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
        for (auto &edge : fromNode->getOutgoingEdges()) {
          if (edge->getIncomingNode() == toNode) {
            dumpEdge(id, edge);
            depIdMap->insert(make_pair(id++, edge));
          }
        }
      } else if (fromId != -1) { // from is specified
        auto node = instIdMap->at(fromId);
        for (auto &edge : node->getOutgoingEdges()) {
          dumpEdge(id, edge);
          depIdMap->insert(make_pair(id++, edge));
        }
      } else if (toId != -1) { // to is specified
        auto node = instIdMap->at(toId);
        for (auto &edge : node->getIncomingEdges()) {
          dumpEdge(id, edge);
          depIdMap->insert(make_pair(id++, edge));
        }
      }

      depIdLookupMap = createDepIdLookupMap(*depIdMap);
      selectedPDG->setDepLookupMap(depIdLookupMap);
      llvm::noelle::DGPrinter::writeClusteredGraph<PDG, Value>(
          "currentPDG.dot",
          selectedPDG.get());
    };

    // remove a dependence
    auto removeFn = [&parser, &depIdMap, &selectedPDG, &selectedSCCDAG]() {
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
    };

    // remove all dependence from a instruction node
    auto removeAllFromInstFn =
        [&parser, &instIdMap, &selectedPDG, &selectedSCCDAG]() {
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
          list<llvm::noelle::DGEdge<Value> *> edgesToRemove;
          for (auto &edge : node->getOutgoingEdges()) {
            edgesToRemove.push_back(edge);
          }

          for (auto &edge : node->getIncomingEdges()) {
            edgesToRemove.push_back(edge);
          }

          for (auto edge : edgesToRemove) {
            selectedPDG->removeEdge(edge);
          }
          // update SCCDAG
          selectedSCCDAG = std::make_unique<SCCDAG>(selectedPDG.get());
        };

    // try to parallelize
    auto parallelizeFn = [&parser, this, &selectedPDG, &selectedLoop]() {
      // int threadBudget = parser.getActionId();
      // if (threadBudget == -1) {
      //   threadBudget = 28;
      // }

      // LoopProfLoad *lpl = &getAnalysis<LoopProfLoad>();
      // auto perf = &getAnalysis<ProfilePerformanceEstimator>();

      // // initialize performance estimator
      // auto psdswp = std::make_shared<PSDSWPCritic>(perf, threadBudget, lpl);
      // auto doall = std::make_shared<DOALLCritic>(perf, threadBudget, lpl);

      // auto check = [](Critic_ptr critic, string name, PDG &pdg, Loop *loop) {
      //   CriticRes res = critic->getCriticisms(pdg, loop);
      //   Criticisms &criticisms = res.criticisms;
      //   unsigned long expSaving = res.expSpeedup;

      //   if (!expSaving) {
      //     outs() << name << " not applicable/profitable\n";
      //   } else {
      //     outs() << name << " applicable, estimated savings: " << expSaving
      //            << "\n";
      //   }
      // };

      // check(doall, "DOALL", *selectedPDG.get(), selectedLoop);
      // check(psdswp, "PSDSWPCritic", *selectedPDG.get(), selectedLoop);
    };

    // modref: create a modref query and (optionally explore the loopaa stack)
    auto modrefFn = [this, &parser, &instIdMap, &selectedLoop]() {
      // int fromId = parser.getFromId();
      // int toId = parser.getToId();

      // if (fromId == -1) {
      //     outs() << "From InstId not set\n";
      //     return;
      // }
      // else {
      //   if (instIdMap->find(fromId) == instIdMap->end()) {
      //     outs() << "From InstId " << fromId<< " not found\n";
      //     return;
      //   }
      // }

      // if (toId == -1) {
      //     outs() << "To InstId not set\n";
      //     return;
      // }
      // else {
      //   if (instIdMap->find(toId) == instIdMap->end()) {
      //     outs() << "To InstId " << toId<< " not found\n";
      //     return;
      //   }
      // }
      // auto fromInst = dyn_cast<Instruction>(instIdMap->at(fromId)->getT());
      // auto toInst = dyn_cast<Instruction>(instIdMap->at(toId)->getT());

      // // TODO: one of the instruction can be a value
      // if (!fromInst || !toInst) {
      //   outs() << "Instructions not found\n";
      //   return;
      // }

      // LoopAA *aa = getAnalysis<LoopAA>().getTopAA();
      // Remedies remeds;

      // if (parser.isVerbose()) {
      //   // TODO: try all combination of analysis and find a setting that the
      //   result is different
      // }
      // else {
      //   auto ret = aa->modref(fromInst,
      //   liberty::LoopAA::TemporalRelation::Same, toInst, selectedLoop,
      //   remeds); outs() << *fromInst << "->" << *toInst << ": (Same)" << ret
      //   << "\n"; ret = aa->modref(fromInst,
      //   liberty::LoopAA::TemporalRelation::Before, toInst, selectedLoop,
      //   remeds); outs() << *fromInst << "->" << *toInst << ": (Before)" <<
      //   ret << "\n"; ret = aa->modref(fromInst,
      //   liberty::LoopAA::TemporalRelation::After, toInst, selectedLoop,
      //   remeds); outs() << *fromInst << "->" << *toInst << ": (After)" << ret
      //   << "\n";
      // }
    };

    auto saveFn = [&parser]() {
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
    };

    switch (parser.getAction()) {
      case ReplAction::Deps:
        depsFn();
        break;
      case ReplAction::Dump:
        dumpFn();
        break;
      case ReplAction::Insts:
        InstsFn();
        break;
      case ReplAction::Remove:
        removeFn();
        break;
      case ReplAction::RemoveAll:
        removeAllFromInstFn();
        break;
      case ReplAction::Parallelize:
        parallelizeFn();
        break;
      case ReplAction::Modref:
        modrefFn();
        break;
      case ReplAction::Save:
        saveFn();
        break;
      default:
        outs() << "SHOULD NOT HAPPEN\n";
        break;
    }
  };

  // execute command history file if specified
  string historyFileName = HistoryFileName;
  if (historyFileName != "") {
    read_history(historyFileName.c_str());
    // DISCUSSION: the last command won't get executed if using 'i <
    // history_length'
    for (int i = history_base; i <= history_length; i++) {
      char *buf = history_get(i)->line;
      string query = (const char *)(buf);
      mainLoop(query);
    }
    clear_history();
  }

  // the main repl while loop
  while (true) {
    if (quit) {
      break;
    }

    stringstream ss;
    ss << "(noelle-repl";
    if (selectLoopId != -1)
      ss << " loop " << selectLoopId;
    ss << ") ";
    char *buf = readline(ss.str().c_str());
    string query = (const char *)(buf);
    if (query.size() > 0) {
      add_history(buf);
      free(buf); // free the buf readline created
    }
    mainLoop(query);
  }

  return modified;
}
