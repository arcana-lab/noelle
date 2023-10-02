/*
 * Copyright 2022 - 2023  Simone Campanoni
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

namespace llvm::noelle {

template <class T>
class DGNode {
public:
  DGNode(int32_t id, T *node);

  T *getT(void) const;

  using nodes_iterator = typename std::vector<DGNode<T> *>::iterator;
  using edges_iterator = typename std::unordered_set<DGEdge<T> *>::iterator;
  using edges_const_iterator =
      typename std::unordered_set<DGEdge<T> *>::const_iterator;

  edges_iterator begin_outgoing_edges() {
    return outgoingEdges.begin();
  }
  edges_iterator end_outgoing_edges() {
    return outgoingEdges.end();
  }
  edges_const_iterator begin_outgoing_edges() const {
    return outgoingEdges.begin();
  }
  edges_const_iterator end_outgoing_edges() const {
    return outgoingEdges.end();
  }

  edges_iterator begin_incoming_edges() {
    return incomingEdges.begin();
  }
  edges_iterator end_incoming_edges() {
    return incomingEdges.end();
  }
  edges_const_iterator begin_incoming_edges() const {
    return incomingEdges.begin();
  }
  edges_const_iterator end_incoming_edges() const {
    return incomingEdges.end();
  }

  std::unordered_set<DGEdge<T> *> getAllEdges(void);

  iterator_range<edges_iterator> getOutgoingEdges(void) {
    return make_range(outgoingEdges.begin(), outgoingEdges.end());
  }

  iterator_range<edges_iterator> getIncomingEdges(void) {
    return make_range(incomingEdges.begin(), incomingEdges.end());
  }

  uint64_t degree(void) const;

  uint64_t outDegree(void) const;

  uint64_t inDegree(void) const;

  void addIncomingEdge(DGEdge<T> *edge);

  void addOutgoingEdge(DGEdge<T> *edge);

  void removeConnectedEdge(DGEdge<T> *edge);

  void removeConnectedNode(DGNode<T> *node);

  std::string toString(void) const;

  raw_ostream &print(raw_ostream &stream);

protected:
  int32_t ID;
  T *theT;
  std::unordered_set<DGEdge<T> *> outgoingEdges;
  std::unordered_set<DGEdge<T> *> incomingEdges;
};

template <class T>
DGNode<T>::DGNode(int32_t id, T *node) : ID{ id },
                                         theT(node) {
  return;
}

template <class T>
T *DGNode<T>::getT(void) const {
  return theT;
}

template <class T>
raw_ostream &DGNode<T>::print(raw_ostream &stream) {
  theT->print(stream);
  return stream;
}

template <class T>
void DGNode<T>::addIncomingEdge(DGEdge<T> *edge) {
  this->incomingEdges.insert(edge);
}

template <class T>
void DGNode<T>::addOutgoingEdge(DGEdge<T> *edge) {
  this->outgoingEdges.insert(edge);
}

template <class T>
void DGNode<T>::removeConnectedEdge(DGEdge<T> *edge) {
  DGNode<T> *node;
  if (outgoingEdges.find(edge) != outgoingEdges.end()) {
    outgoingEdges.erase(edge);
    node = edge->getDstNode();
  } else {
    incomingEdges.erase(edge);
    node = edge->getSrcNode();
  }
}

template <class T>
void DGNode<T>::removeConnectedNode(DGNode<T> *node) {
  std::unordered_set<DGEdge<T> *> outgoingEdgesToRemove{};
  for (auto edge : outgoingEdges) {
    if (edge->getDstNode() == node) {
      outgoingEdgesToRemove.insert(edge);
    }
  }
  for (auto edge : outgoingEdgesToRemove) {
    outgoingEdges.erase(edge);
  }

  std::unordered_set<DGEdge<T> *> incomingEdgesToRemove{};
  for (auto edge : incomingEdges) {
    if (edge->getSrcNode() == node) {
      incomingEdgesToRemove.insert(edge);
    }
  }
  for (auto edge : incomingEdgesToRemove) {
    incomingEdges.erase(edge);
  }
}

template <class T>
std::unordered_set<DGEdge<T> *> DGNode<T>::getAllEdges(void) {
  std::unordered_set<DGEdge<T> *> allConnectedEdges{ outgoingEdges.begin(),
                                                     outgoingEdges.end() };
  allConnectedEdges.insert(incomingEdges.begin(), incomingEdges.end());
  return allConnectedEdges;
}

template <class T>
uint64_t DGNode<T>::degree(void) const {
  return outgoingEdges.size() + incomingEdges.size();
}

template <class T>
uint64_t DGNode<T>::outDegree(void) const {
  return outgoingEdges.size();
}

template <class T>
uint64_t DGNode<T>::inDegree(void) const {
  return incomingEdges.size();
}

} // namespace llvm::noelle
