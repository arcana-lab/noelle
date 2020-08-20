/*
 * Copyright 2019 - 2020 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"
#include "CallGraph.hpp"

namespace llvm::noelle {

  CallGraph::CallGraph (Module &M)
    : m{M},
      scccag{nullptr}
    {
    return ;
  }

  CallGraph::CallGraph (Module &M, PTACallGraph *callGraph)
    : m{M},
      scccag{nullptr}
    {

    /*
     * Create the function nodes.
     */
    for (auto &F: M){

      /*
       * Create a node for the current function.
       */
      auto newNode = new CallGraphFunctionNode(F);
      this->functions[&F] = newNode;
    }

    /*
     * Create the edges.
     */
    for (auto &F : M){

      /*
       * Fetch the node of the current function.
       */
      auto fromNode = this->functions[&F];

      /*
       * Add the edges from this function.
       */
      for (auto &inst : instructions(&F)){

        /*
         * Handle call instructions.
         */
        if (auto callInst = dyn_cast<CallInst>(&inst)){
          this->handleCallInstruction(fromNode, callInst, callGraph);
          continue ;
        }

        /*
         * Handle invoke instructions.
         */
        if (auto callInst = dyn_cast<InvokeInst>(&inst)){
          this->handleCallInstruction(fromNode, callInst, callGraph);
          continue ;
        }
      }
    }

    return ;
  }

  std::unordered_set<CallGraphFunctionNode *> CallGraph::getFunctionNodes (void) const {
    std::unordered_set<CallGraphFunctionNode *> s;

    for (auto pair : this->functions){
      s.insert(pair.second);
    }

    return s;
  }

  std::unordered_set<CallGraphEdge *> CallGraph::getEdges (void) const {
    return this->edges;
  }

  CallGraphFunctionNode * CallGraph::getEntryNode (void) const {
    Function *f = m.getFunction("main");
    return this->getFunctionNode(f);
  }

  CallGraphFunctionNode * CallGraph::getFunctionNode (Function *f) const {
    if (this->functions.find(f) == this->functions.end()){
      return nullptr;
    }
    auto n = this->functions.at(f);

    return n;
  }

  void CallGraph::handleCallInstruction (CallGraphFunctionNode *fromNode, CallBase *callInst, PTACallGraph *callGraph){

    /*
     * Fetch the callee.
     */
    auto callee = callInst->getCalledFunction();
    if (callee != nullptr){

      /*
       * Add the edge if it doesn't exist.
       */
      this->fetchOrCreateEdge(fromNode, callInst, *callee, true);

      return ;
    }

    /*
     * The callee is unknown.
     */
    if (isa<CallInst>(callInst)){
      auto callInstCast = cast<CallInst>(callInst);
      if (!callGraph->hasIndCSCallees(callInstCast)) {
        return ;
      }

      /*
       * Iterate over the possible callees.
       */
      auto callees = callGraph->getIndCSCallees(callInstCast);
      for (auto &callee : callees) {

        /*
         * Add the edge if it doesn't exist.
         */
        auto nonConstCallee = const_cast<Function *>(callee);
        this->fetchOrCreateEdge(fromNode, callInst, *nonConstCallee, false);
      }
    }

    return ;
  }
          
  SCCCAG * CallGraph::getSCCCAG (void) {

    /*
     * Check if we have already computed it.
     */
    if (this->scccag != nullptr){
      return this->scccag;
    }

    /*
     * Compute the SCCCAG.
     */
    this->scccag = new SCCCAG(this);

    return this->scccag;
  }

  bool CallGraph::doesItBelongToASCC (Function *f) {

    /*
     * Fetch the SCCCAG.
     */
    auto localAG = this->getSCCCAG();

    /*
     * Fetch the SCCCAG node of @f.
     */
    auto callGraphNode = this->getFunctionNode(f);
    assert(callGraphNode != nullptr);
    auto localAGNode = localAG->getNode(callGraphNode);
    assert(localAGNode != nullptr);

    /*
     * Check if the node belongs to an SCC.
     */
    if (localAGNode->isAnSCC()){
      return true;
    }

    /*
     * The node doesn't belong to an SCC.
     */
    return false;
  }

  std::unordered_map<Function *, CallGraph *> CallGraph::getIslands (void) const {
    std::unordered_map<Function *, CallGraph *> islands{};

    /*
     * Identify the islands in the call graph by inspecting call/invoke instructions.
     */
    this->identifyCallGraphIslandsByCallInstructions(islands);

    /*
     * Merge islands due to escaped functions.
     */
    this->mergeCallGraphIslandsForEscapedFunctions(islands);

    return islands;
  }

  void CallGraph::mergeCallGraphIslandsForEscapedFunctions (std::unordered_map<Function *, CallGraph *> &islands) const {
    
    /*
     * Identify the functions that are stored in memory.
     */
    for (auto fPair : this->functions){
      auto f = fPair.first;

      /*
       * Check every use of the current function.
       */
      for (auto &use : f->uses()){

        /*
         * Fetch the next instruction.
         */
        auto user = use.getUser();
        if (!isa<Instruction>(user)){
          continue ;
        }
        auto inst = cast<Instruction>(user);

        /*
         * Ignore call instructions that invoke @f.
         */
        if (isa<CallInst>(user)){
          auto callInstUser = cast<CallInst>(user);
          if (callInstUser->getCalledFunction() == f){
            continue ;
          }

        } else if (isa<InvokeInst>(user)){
          auto callInstUser = cast<InvokeInst>(user);
          if (callInstUser->getCalledFunction() == f){
            continue ;
          }
        }

        /*
         * The function escapes.
         * Merge the island that holds the current instruction that stores the current function and the function's one.
         */
        auto instFunction = inst->getFunction();
        auto instIsland = islands[instFunction];
        assert(instIsland != nullptr);
        auto fIsland = islands[f];
        assert(fIsland != nullptr);
        if (instIsland == fIsland){
          continue ;
        }
        std::vector<std::pair<Function *, CallGraphFunctionNode *>> toDelete{};
        for (auto instIsland_function_pair : instIsland->functions){

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
        for (auto fNPair : toDelete){
          instIsland->functions[fNPair.first] = nullptr;
          instIsland->functions.erase(fNPair.first);
        }
        delete instIsland;
      }
    }

    return ;
  }

  void CallGraph::identifyCallGraphIslandsByCallInstructions (std::unordered_map<Function *, CallGraph *> &islands) const {

    /*
     * Define the code to execute every time we add a function to an island.
     */
    std::unordered_set<Function *> visited{};
    auto addToIsland = [&islands, &visited] (Function *f, CallGraph *island) {

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

      return ;
    };

    /*
     * Compute the islands.
     */
    for (auto fPair : this->functions){
      auto f = fPair.first;
      auto n = fPair.second;

      /*
       * Check if the current function has been visited already.
       */
      if (visited.find(f) != visited.end()){
        continue ;
      }

      /*
       * The current function hasn't been visited yet.
       *
       * Create a new island that includes this function.
       */
      auto newIsland = new CallGraph(this->m);
      addToIsland(f, newIsland);

      /*
       * Bring every reachable functions from the current one inside the new island.
       */
      std::stack<CallGraphFunctionNode *> todos{};
      todos.push(n);
      while (!todos.empty()){

        /*
         * Fetch the current element.
         */
        auto currentNode = todos.top();
        todos.pop();
        assert(this->getFunctionNode(currentNode->getFunction()) == currentNode);

        /*
         * Iterate over the edges.
         */
        for (auto edge : currentNode->getEdges()){

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
          if (newIsland->getFunctionNode(calleeFunction) == nullptr){
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
          if (newIsland->getFunctionNode(callerFunction) == nullptr){
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

    return ;
  }

  CallGraphFunctionFunctionEdge * CallGraph::fetchOrCreateEdge (CallGraphFunctionNode *fromNode, CallBase *callInst, Function & callee, bool isMust){

    /*
     * Fetch the callee node.
     */
    assert(this->functions.find(&callee) != this->functions.end());
    auto toNode = this->functions.at(&callee);

    /*
     * Create the sub-edge.
     */
    auto instNode = this->instructionNodes[callInst];
    if (instNode == nullptr){
      instNode = new CallGraphInstructionNode(callInst);
      this->instructionNodes[callInst] = instNode;
    }
    auto subEdge = new CallGraphInstructionFunctionEdge(instNode, toNode, isMust);

    /*
     * Check if the edge already exists.
     */
    auto existingEdge = fromNode->getCallEdgeTo(toNode);
    if (existingEdge == nullptr){

      /*
       * The edge from @fromNode to @toNode doesn't exist yet.
       *
       * Create a new edge.
       */
      auto newEdge = new CallGraphFunctionFunctionEdge(fromNode, toNode, isMust);
      this->edges.insert(newEdge);

      /*
       * Add the new edge.
       */
      fromNode->addOutgoingEdge(newEdge);
      toNode->addIncomingEdge(newEdge);

      /*
       * Add the sub-edge.
       */
      newEdge->addSubEdge(subEdge);

      return newEdge;
    }

    /*
     * The edge from @fromNode to @toNode already exists.
     *
     * Check if we need to change its flag to must.
     */
    if (isMust){

      /*
       * Change the flag to must.
       */
      existingEdge->setMust();
    }

      /*
     * Add the sub-edge.
     */
    existingEdge->addSubEdge(subEdge);

    return existingEdge;
  }

}
