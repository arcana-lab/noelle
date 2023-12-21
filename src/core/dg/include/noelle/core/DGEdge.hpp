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
#ifndef NOELLE_SRC_CORE_DG_DGEDGE_H_
#define NOELLE_SRC_CORE_DG_DGEDGE_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/DGNode.hpp"

namespace arcana::noelle {

enum DataDependenceType { DG_DATA_NONE, DG_DATA_RAW, DG_DATA_WAR, DG_DATA_WAW };

/*
 * This is the top of the class hierarchy that organizes dependences.
 *
 * https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html
 */
template <class T, class SubT>
class DGEdge {
public:

  /*
   * Concrete sub-classes.
   */
  enum DependenceKind {
    CONTROL_DEPENDENCE,

    FIRST_DATA_DEPENDENCE,
    VARIABLE_DEPENDENCE,
    MEMORY_DEPENDENCE,
    LAST_DATA_DEPENDENCE,

    UNDEFINED_DEPENDENCE
  };

  /*
   * No public constructors.
   * Only objects of sub-classes can be allocated.
   */
  DGEdge() = delete;

  using edges_iterator =
      typename std::unordered_set<DGEdge<SubT, SubT> *>::iterator;
  using edges_const_iterator =
      typename std::unordered_set<DGEdge<SubT, SubT> *>::const_iterator;

  edges_iterator begin_sub_edges() {
    return subEdges.begin();
  }
  edges_iterator end_sub_edges() {
    return subEdges.end();
  }
  edges_const_iterator begin_sub_edges() const {
    return subEdges.begin();
  }
  edges_const_iterator end_sub_edges() const {
    return subEdges.end();
  }

  iterator_range<edges_iterator> getSubEdges() {
    return make_range(subEdges.begin(), subEdges.end());
  }

  std::pair<DGNode<T> *, DGNode<T> *> getNodePair() const {
    return std::make_pair(from, to);
  }

  void setNodePair(DGNode<T> *from, DGNode<T> *to) {
    this->from = from;
    this->to = to;
  }

  DGNode<T> *getSrcNode(void) const;

  DGNode<T> *getDstNode(void) const;

  T *getSrc(void) const;

  T *getDst(void) const;

  bool isMemoryDependence() const {
    return memory;
  }

  bool isMustDependence() const {
    return must;
  }

  bool isRAWDependence() const {
    return dataDepType == DG_DATA_RAW;
  }

  bool isWARDependence() const {
    return dataDepType == DG_DATA_WAR;
  }

  bool isWAWDependence() const {
    return dataDepType == DG_DATA_WAW;
  }

  DataDependenceType dataDependenceType() const {
    return dataDepType;
  }

  void setMemMustType(bool mem, bool must, DataDependenceType dataDepType);

  void setLoopCarried(bool lc) {
    isLoopCarried = lc;
  }

  void setEdgeAttributes(bool mem,
                         bool must,
                         std::string str,
                         bool lc) {
    setMemMustType(mem, must, stringToDataDep(str));
    setLoopCarried(lc);

    return;
  }

  bool isLoopCarriedDependence() const;

  void addSubEdge(DGEdge<SubT, SubT> *edge);

  void removeSubEdge(DGEdge<SubT, SubT> *edge);

  void removeSubEdges(void);

  std::string dataDepToString(void);

  virtual std::string toString(void) = 0;

  raw_ostream &print(raw_ostream &stream, std::string linePrefix = "");

  DependenceKind getKind(void) const;

  static DataDependenceType stringToDataDep(std::string &str);

protected:
  DGNode<T> *from;
  DGNode<T> *to;
  std::unordered_set<DGEdge<SubT, SubT> *> subEdges;
  bool isLoopCarried;
  DataDependenceType dataDepType;
  bool memory;
  bool must;

  DGEdge(DependenceKind k, DGNode<T> *src, DGNode<T> *dst);
  DGEdge(const DGEdge<T, SubT> &edgeToCopy);

private:
  DependenceKind kind;
};

template <class T, class SubT>
DGEdge<T, SubT>::DGEdge(DependenceKind k, DGNode<T> *src, DGNode<T> *dst)
  : from(src),
    to(dst),
    subEdges{},
    memory{ false },
    must{ false },
    isLoopCarried(false),
    dataDepType{ DG_DATA_NONE },
    kind{k} {
  return;
}

template <class T, class SubT>
DGEdge<T, SubT>::DGEdge(const DGEdge<T, SubT> &edgeToCopy){
  auto nodePair = edgeToCopy.getNodePair();
  from = nodePair.first;
  to = nodePair.second;
  setMemMustType(edgeToCopy.isMemoryDependence(),
                 edgeToCopy.isMustDependence(),
                 edgeToCopy.dataDependenceType());
  setLoopCarried(edgeToCopy.isLoopCarriedDependence());
  for (auto subEdge : edgeToCopy.subEdges){
    addSubEdge(subEdge);
  }
  this->kind = edgeToCopy.kind;

  return ;
}

template <class T, class SubT>
void DGEdge<T, SubT>::removeSubEdge(DGEdge<SubT, SubT> *edge) {
  subEdges.erase(edge);

  return;
}

template <class T, class SubT>
void DGEdge<T, SubT>::removeSubEdges(void) {
  subEdges.clear();
  setLoopCarried(false);

  return;
}

template <class T, class SubT>
void DGEdge<T, SubT>::setMemMustType(bool mem,
                                     bool must,
                                     DataDependenceType dataDepType) {
  this->memory = mem;
  this->must = must;
  this->dataDepType = dataDepType;
}

template <class T, class SubT>
std::string DGEdge<T, SubT>::dataDepToString(void) {
  if (this->isRAWDependence())
    return "RAW";
  else if (this->isWARDependence())
    return "WAR";
  else if (this->isWAWDependence())
    return "WAW";
  else
    return "NONE";
}

template <class T, class SubT>
raw_ostream &DGEdge<T, SubT>::print(raw_ostream &stream,
                                    std::string linePrefix) {
  from->print(stream << linePrefix << "From:\t") << "\n";
  to->print(stream << linePrefix << "To:\t") << "\n";
  stream << linePrefix << this->toString();
  return stream;
}

template <class T, class SubT>
void DGEdge<T, SubT>::addSubEdge(DGEdge<SubT, SubT> *edge) {
  subEdges.insert(edge);
  isLoopCarried |= edge->isLoopCarriedDependence();
  return;
}

template <class T, class SubT>
DataDependenceType DGEdge<T, SubT>::stringToDataDep(std::string &str) {
  if (str == "RAW")
    return DG_DATA_RAW;
  else if (str == "WAR")
    return DG_DATA_WAR;
  else if (str == "WAW")
    return DG_DATA_WAW;
  else
    return DG_DATA_NONE;
}

template <class T, class SubT>
DGNode<T> *DGEdge<T, SubT>::getSrcNode(void) const {
  return from;
}

template <class T, class SubT>
DGNode<T> *DGEdge<T, SubT>::getDstNode(void) const {
  return to;
}

template <class T, class SubT>
T *DGEdge<T, SubT>::getSrc(void) const {
  return from->getT();
}

template <class T, class SubT>
T *DGEdge<T, SubT>::getDst(void) const {
  return to->getT();
}

template <class T, class SubT>
bool DGEdge<T, SubT>::isLoopCarriedDependence() const {
  return isLoopCarried;
}
  
template <class T, class SubT>
typename DGEdge<T, SubT>::DependenceKind DGEdge<T, SubT>::getKind(void) const{
  return this->kind;
}

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_DG_DGEDGE_H_
