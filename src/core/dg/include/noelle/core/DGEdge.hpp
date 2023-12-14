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

template <class T, class SubT>
class DGEdge {
public:
  DGEdge(DGNode<T> *src, DGNode<T> *dst);

  DGEdge(const DGEdge<T, SubT> &oldEdge);

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

  bool isControlDependence() const {
    return isControl;
  }

  bool isDataDependence() const {
    return !isControl;
  }

  DataDependenceType dataDependenceType() const {
    return dataDepType;
  }

  bool isRemovableDependence() const {
    return isRemovable;
  }

  std::optional<SetOfRemedies> getRemedies() const {
    return (remeds) ? std::make_optional<SetOfRemedies>(*remeds) : std::nullopt;
  }

  void setControl(bool ctrl) {
    isControl = ctrl;
  }

  void setMemMustType(bool mem, bool must, DataDependenceType dataDepType);

  void setLoopCarried(bool lc) {
    isLoopCarried = lc;
  }

  void setRemedies(std::optional<SetOfRemedies> R) {
    if (R) {
      remeds = std::make_unique<SetOfRemedies>(*R);
      isRemovable = true;
    }
  }

  void addRemedies(const Remedies_ptr &R) {
    if (!remeds) {
      remeds = std::make_unique<SetOfRemedies>();
      isRemovable = true;
    }
    remeds->insert(R);
  }

  void setRemovable(bool rem) {
    isRemovable = rem;
  }

  void setEdgeAttributes(bool mem,
                         bool must,
                         std::string str,
                         bool ctrl,
                         bool lc,
                         bool rm) {
    setMemMustType(mem, must, stringToDataDep(str));
    setControl(ctrl);
    setLoopCarried(lc);
    setRemovable(rm);

    return;
  }

  bool isLoopCarriedDependence() const;

  void addSubEdge(DGEdge<SubT, SubT> *edge);

  void removeSubEdge(DGEdge<SubT, SubT> *edge);

  void removeSubEdges(void);

  std::string toString();

  raw_ostream &print(raw_ostream &stream, std::string linePrefix = "");

  std::string dataDepToString();

  static DataDependenceType stringToDataDep(std::string &str);

protected:
  DGNode<T> *from;
  DGNode<T> *to;
  std::unordered_set<DGEdge<SubT, SubT> *> subEdges;
  bool memory;
  bool must;
  bool isControl;
  bool isLoopCarried;
  bool isRemovable;
  DataDependenceType dataDepType;
  SetOfRemedies_ptr remeds;
};

template <class T, class SubT>
DGEdge<T, SubT>::DGEdge(DGNode<T> *src, DGNode<T> *dst)
  : from(src),
    to(dst),
    subEdges{},
    memory{ false },
    must{ false },
    isControl(false),
    isLoopCarried(false),
    isRemovable(false),
    dataDepType{ DG_DATA_NONE },
    remeds(nullptr) {
  return;
}

template <class T, class SubT>
DGEdge<T, SubT>::DGEdge(const DGEdge<T, SubT> &oldEdge) {
  auto nodePair = oldEdge.getNodePair();
  from = nodePair.first;
  to = nodePair.second;
  setMemMustType(oldEdge.isMemoryDependence(),
                 oldEdge.isMustDependence(),
                 oldEdge.dataDependenceType());
  setControl(oldEdge.isControlDependence());
  setLoopCarried(oldEdge.isLoopCarriedDependence());
  setRemovable(oldEdge.isRemovableDependence());
  setRemedies(oldEdge.getRemedies());
  for (auto subEdge : oldEdge.subEdges)
    addSubEdge(subEdge);
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
  remeds = nullptr;
  setRemovable(false);

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
std::string DGEdge<T, SubT>::dataDepToString() {
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
std::string DGEdge<T, SubT>::toString() {
  if (this->subEdges.size() > 0) {
    std::string edgesStr;
    raw_string_ostream ros(edgesStr);
    for (auto edge : this->subEdges)
      ros << edge->toString();
    return ros.str();
  }
  std::string edgeStr;
  raw_string_ostream ros(edgeStr);
  ros << "Attributes: ";
  if (this->isLoopCarried) {
    ros << "Loop-carried ";
  }
  if (this->isControlDependence()) {
    ros << "Control ";

  } else {
    ros << "Data ";
    ros << this->dataDepToString();
    ros << (must ? " (must)" : " (may)");
    ros << (memory ? " from memory " : "");
  }
  ros << "\n";
  ros.flush();
  return edgeStr;
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
  if (edge->isRemovableDependence()
      && (subEdges.size() == 1 || this->isRemovableDependence())) {
    isRemovable = true;
    if (auto optional_remeds = edge->getRemedies()) {
      for (auto &r : *(optional_remeds))
        this->addRemedies(r);
    }
  } else {
    remeds = nullptr;
    isRemovable = false;
  }

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

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_DG_DGEDGE_H_
