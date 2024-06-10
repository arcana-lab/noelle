/*
 * Copyright 2019 - 2020 Simone Campanoni
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
#include "arcana/noelle/core/SystemHeaders.hpp"
#include "noelle/core/CallGraph.hpp"

namespace arcana::noelle {

CallGraph::CallGraph(Module &M) : m{ M } {

  return;
}

CallGraph::CallGraph(
    Module &M,
    std::function<bool(CallInst *)> hasIndCSCallees,
    std::function<const std::set<const Function *>(CallInst *)> getIndCSCallees)
  : m{ M } {

  /*
   * Create the function nodes.
   */
  for (auto &F : M) {

    /*
     * Create a node for the current function.
     */
    auto newNode = new CallGraphFunctionNode(F);
    this->functions[&F] = newNode;
  }

  /*
   * Create the edges.
   */
  for (auto &F : M) {

    /*
     * Fetch the node of the current function.
     */
    auto fromNode = this->functions[&F];

    /*
     * Add the edges from this function.
     */
    for (auto &inst : instructions(&F)) {

      /*
       * Handle call instructions.
       */
      if (auto callInst = dyn_cast<CallInst>(&inst)) {
        this->handleCallInstruction(fromNode,
                                    callInst,
                                    hasIndCSCallees,
                                    getIndCSCallees);
        continue;
      }

      /*
       * Handle invoke instructions.
       */
      if (auto callInst = dyn_cast<InvokeInst>(&inst)) {
        this->handleCallInstruction(fromNode,
                                    callInst,
                                    hasIndCSCallees,
                                    getIndCSCallees);
        continue;
      }
    }
  }

  return;
}

std::unordered_set<CallGraphFunctionNode *> CallGraph::getFunctionNodes(
    bool mustHaveBody) const {
  std::unordered_set<CallGraphFunctionNode *> s;

  for (auto pair : this->functions) {
    auto f = pair.first;
    if (mustHaveBody && f->empty()) {
      continue;
    }
    s.insert(pair.second);
  }

  return s;
}

CallGraphFunctionNode *CallGraph::getEntryNode(void) const {
  Function *f = m.getFunction("main");
  return this->getFunctionNode(f);
}

CallGraphFunctionNode *CallGraph::getFunctionNode(Function *f) const {
  if (this->functions.find(f) == this->functions.end()) {
    return nullptr;
  }
  auto n = this->functions.at(f);

  return n;
}

void CallGraph::handleCallInstruction(
    CallGraphFunctionNode *fromNode,
    CallBase *callInst,
    std::function<bool(CallInst *)> hasIndCSCallees,
    std::function<const std::set<const Function *>(CallInst *)>
        getIndCSCallees) {

  /*
   * Fetch the callee.
   */
  auto callee = callInst->getCalledFunction();
  if (callee != nullptr) {

    /*
     * Add the edge if it doesn't exist.
     */
    this->fetchOrCreateEdge(fromNode, callInst, *callee, true);

    return;
  }

  /*
   * The callee is unknown.
   */
  if (isa<CallInst>(callInst)) {
    auto callInstCast = cast<CallInst>(callInst);
    if (!hasIndCSCallees(callInstCast)) {
      return;
    }

    /*
     * Iterate over the possible callees.
     */
    auto callees = getIndCSCallees(callInstCast);
    for (auto &callee : callees) {

      /*
       * Add the edge if it doesn't exist.
       */
      auto nonConstCallee = const_cast<Function *>(callee);
      this->fetchOrCreateEdge(fromNode, callInst, *nonConstCallee, false);
    }
  }

  return;
}

std::unordered_map<Function *, CallGraph *> CallGraph::getIslands(void) const {
  std::unordered_map<Function *, CallGraph *> islands{};

  /*
   * Identify the islands in the call graph by inspecting call/invoke
   * instructions.
   */
  this->identifyCallGraphIslandsByCallInstructions(islands);

  /*
   * Merge islands due to escaped functions.
   */
  this->mergeCallGraphIslandsForEscapedFunctions(islands);

  return islands;
}

void CallGraph::mergeCallGraphIslandsForEscapedFunctions(
    std::unordered_map<Function *, CallGraph *> &islands) const {

  /*
   * Identify the functions that are stored in memory.
   */
  for (auto fPair : this->functions) {
    auto f = fPair.first;

    /*
     * Check every use of the current function.
     */
    for (auto &use : f->uses()) {

      /*
       * Fetch the next instruction.
       */
      auto user = use.getUser();
      if (!isa<Instruction>(user)) {
        continue;
      }
      auto inst = cast<Instruction>(user);

      /*
       * Ignore call instructions that invoke @f.
       */
      if (isa<CallInst>(user)) {
        auto callInstUser = cast<CallInst>(user);
        if (callInstUser->getCalledFunction() == f) {
          continue;
        }

      } else if (isa<InvokeInst>(user)) {
        auto callInstUser = cast<InvokeInst>(user);
        if (callInstUser->getCalledFunction() == f) {
          continue;
        }
      }

      /*
       * The function escapes.
       * Merge the island that holds the current instruction that stores the
       * current function and the function's one.
       */
      auto instFunction = inst->getFunction();
      auto instIsland = islands[instFunction];
      assert(instIsland != nullptr);
      auto fIsland = islands[f];
      assert(fIsland != nullptr);
      if (instIsland == fIsland) {
        continue;
      }
      std::vector<std::pair<Function *, CallGraphFunctionNode *>> toDelete{};
      for (auto instIsland_function_pair : instIsland->functions) {

        /*
         * Fetch the node from the island that is going to be deleted.
         */
        auto instIsland_function = instIsland_function_pair.first;
        auto instIsland_functionNode = instIsland_function_pair.second;

        /*
         * Move the node to the final island.
         */
        fIsland->functions[instIsland_function] = instIsland_functionNode;
        islands[instIsland_function] = fIsland;
        toDelete.push_back(instIsland_function_pair);
      }

      /*
       * Delete the island that includes the current store instruction.
       */
      for (auto fNPair : toDelete) {
        instIsland->functions[fNPair.first] = nullptr;
        instIsland->functions.erase(fNPair.first);
      }
      delete instIsland;
    }
  }

  return;
}

void CallGraph::identifyCallGraphIslandsByCallInstructions(
    std::unordered_map<Function *, CallGraph *> &islands) const {

  /*
   * Define the code to execute every time we add a function to an island.
   */
  std::unordered_set<Function *> visited{};
  auto addToIsland = [&islands, &visited](Function *f, CallGraph *island) {
    /*
     * Create a new node.
     */
    auto newNode = new CallGraphFunctionNode(*f);

    /*
     * Add the new node to the island.
     */

    island->functions[f] = newNode;

    /*
     * Tag the function @f as visited.
     */
    visited.insert(f);

    /*
     * Keep track of the function -> island mapping.
     */
    islands[f] = island;

    return;
  };

  /*
   * Compute the islands.
   */
  for (auto fPair : this->functions) {
    auto f = fPair.first;
    auto n = fPair.second;

    /*
     * Check if the current function has been visited already.
     */
    if (visited.find(f) != visited.end()) {
      continue;
    }

    /*
     * The current function hasn't been visited yet.
     *
     * Create a new island that includes this function.
     */
    auto newIsland = new CallGraph(this->m);
    addToIsland(f, newIsland);

    /*
     * Bring every reachable functions from the current one inside the new
     * island.
     */
    std::stack<CallGraphFunctionNode *> todos{};
    todos.push(n);
    while (!todos.empty()) {

      /*
       * Fetch the current element.
       */
      auto currentNode = todos.top();
      todos.pop();
      assert(this->getFunctionNode(currentNode->getFunction()) == currentNode);

      /*
       * Iterate over the edges.
       */
      for (auto edge : this->getEdges(currentNode)) {

        /*
         * Fetch the calleer.
         */
        auto callerNode = edge->getCaller();
        auto callerFunction = callerNode->getFunction();

        /*
         * Fetch the callee.
         */
        auto calleeNode = edge->getCallee();
        auto calleeFunction = calleeNode->getFunction();

        /*
         * Check if the callee has been visited already.
         */
        if (newIsland->getFunctionNode(calleeFunction) == nullptr) {
          assert(visited.find(calleeFunction) == visited.end());

          /*
           * The callee hasn't been visited yet.
           *
           * Copy the callee into the current island.
           */
          addToIsland(calleeFunction, newIsland);

          /*
           * Tag the callee to be evaluated.
           */
          todos.push(calleeNode);
        }

        /*
         * Check if the caller has been visited already.
         */
        if (newIsland->getFunctionNode(callerFunction) == nullptr) {
          assert(visited.find(callerFunction) == visited.end());

          /*
           * The caller hasn't been visited yet.
           *
           * Copy the caller into the current island.
           */
          addToIsland(callerFunction, newIsland);

          /*
           * Tag the caller to be evaluated.
           */
          todos.push(callerNode);
        }
      }
    }
  }

  return;
}

CallGraphFunctionFunctionEdge *CallGraph::fetchOrCreateEdge(
    CallGraphFunctionNode *fromNode,
    CallBase *callInst,
    Function &callee,
    bool isMust) {

  /*
   * Fetch the callee node.
   */
  assert(this->functions.find(&callee) != this->functions.end());
  auto toNode = this->functions.at(&callee);

  /*
   * Create the sub-edge.
   */
  auto instNode = this->instructionNodes[callInst];
  if (instNode == nullptr) {
    instNode = new CallGraphInstructionNode(callInst);
    this->instructionNodes[callInst] = instNode;
  }
  auto subEdge = new CallGraphInstructionFunctionEdge(instNode, toNode, isMust);

  /*
   * Check if the edge already exists.
   */
  auto existingEdge = this->getEdge(fromNode, toNode);
  if (existingEdge == nullptr) {

    /*
     * The edge from @fromNode to @toNode doesn't exist yet.
     *
     * Create a new edge.
     */
    auto newEdge = new CallGraphFunctionFunctionEdge(fromNode, toNode, isMust);

    /*
     * Add the new edge.
     */
    auto &tmp = this->outgoingEdges[fromNode];
    tmp[toNode] = newEdge;
    auto &tmp2 = this->incomingEdges[toNode];
    tmp2[fromNode] = newEdge;

    existingEdge = newEdge;
  }
  assert(existingEdge != nullptr);

  /*
   * The edge from @fromNode to @toNode exists at this point.
   *
   * Add the sub-edge.
   */
  existingEdge->addSubEdge(subEdge);

  /*
   * Check if we need to change its flag to must.
   */
  if (isMust) {

    /*
     * Change the flag to must.
     */
    existingEdge->setMust();
  }

  return existingEdge;
}

bool CallGraph::canFunctionEscape(Function *f) const {

  /*
   * Check all uses of @f
   */
  for (auto &use : f->uses()) {

    /*
     * Fetch the next user that is an instruction.
     */
    auto user = use.getUser();
    if (!isa<Instruction>(user)) {
      continue;
    }
    auto userInst = cast<Instruction>(user);

    /*
     * Handle call and invoke instructions.
     */
    if (auto callInst = dyn_cast<CallBase>(userInst)) {

      /*
       * Check what the reference of @f is used for.
       */
      auto canEscape = false;
      for (auto argID = 0u; argID < callInst->getNumArgOperands(); argID++) {
        auto arg = callInst->getArgOperand(argID);
        if (arg == f) {
          canEscape = true;
          break;
        }
      }
      if (canEscape) {
        return true;
      }

      /*
       * The current reference to @f is for declaring the callee of @callInst.
       * Hence, @f doesn't escape because of this use.
       */
      continue;
    }

    /*
     * Handle store instructions.
     */
    if (isa<StoreInst>(userInst)) {
      return true;
    }
  }

  return true;
}

CallGraphFunctionFunctionEdge *CallGraph::getEdge(
    CallGraphFunctionNode *from,
    CallGraphFunctionNode *to) const {

  /*
   * Fetch the set of edges from @from.
   */
  if (this->outgoingEdges.find(from) == this->outgoingEdges.end()) {
    return nullptr;
  }
  auto &tmp = this->outgoingEdges.at(from);

  /*
   * Fetch the edge to @to
   */
  if (tmp.find(to) == tmp.end()) {
    return nullptr;
  }
  auto e = tmp.at(to);

  return e;
}

std::unordered_set<CallGraphFunctionFunctionEdge *> CallGraph::getIncomingEdges(
    CallGraphFunctionNode *node) const {
  std::unordered_set<CallGraphFunctionFunctionEdge *> s;

  if (this->incomingEdges.find(node) == this->incomingEdges.end()) {
    return s;
  }
  auto &tmp = this->incomingEdges.at(node);
  for (auto p : tmp) {
    s.insert(p.second);
  }

  return s;
}

std::unordered_set<CallGraphFunctionFunctionEdge *> CallGraph::getOutgoingEdges(
    CallGraphFunctionNode *node) const {
  std::unordered_set<CallGraphFunctionFunctionEdge *> s;

  if (this->outgoingEdges.find(node) == this->outgoingEdges.end()) {
    return s;
  }
  auto &tmp = this->outgoingEdges.at(node);
  for (auto p : tmp) {
    s.insert(p.second);
  }

  return s;
}

std::unordered_set<CallGraphFunctionFunctionEdge *> CallGraph::getEdges(
    CallGraphFunctionNode *node) const {
  auto s0 = this->getIncomingEdges(node);
  auto s1 = this->getOutgoingEdges(node);
  s0.insert(s1.begin(), s1.end());

  return s0;
}

void CallGraph::removeSubEdge(CallGraphFunctionFunctionEdge *e,
                              CallGraphInstructionFunctionEdge *se) {

  /*
   * Remove the subedge
   */
  e->removeSubEdge(se);

  /*
   * Check if the edge is now empty
   */
  if (e->getNumberOfSubEdges() == 0) {

    /*
     * This edge is meaningless as it has no sub-edges.
     *
     * Remove it from the outgoingEdges map.
     */
    auto caller = e->getCaller();
    auto callee = e->getCallee();
    auto &tmp = this->outgoingEdges.at(caller);
    assert(tmp.at(callee) == e);
    tmp.erase(callee);

    /*
     * Remove it from the incomingEdges map.
     */
    auto &tmp2 = this->incomingEdges.at(callee);
    assert(tmp2.at(caller) == e);
    tmp2.erase(caller);

    /*
     * Destroy the edge.
     */
    delete e;
  }

  return;
}

} // namespace arcana::noelle
