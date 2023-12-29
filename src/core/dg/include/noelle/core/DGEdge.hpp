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

    FIRST_MEMORY_DEPENDENCE,
    MAY_MEMORY_DEPENDENCE,
    MUST_MEMORY_DEPENDENCE,
    LAST_MEMORY_DEPENDENCE,

    LAST_DATA_DEPENDENCE,

    UNDEFINED_DEPENDENCE
  };

  /*
   * No public constructors.
   * Only objects of sub-classes can be allocated.
   */
  DGEdge() = delete;

  std::unordered_set<DGEdge<SubT, SubT> *> getSubEdges(void) const;

  uint64_t getNumberOfSubEdges(void) const;

  DGNode<T> *getSrcNode(void) const;

  DGNode<T> *getDstNode(void) const;

  T *getSrc(void) const;

  T *getDst(void) const;

  void setSrcNode(DGNode<T> *from);

  void setDstNode(DGNode<T> *to);

  bool isLoopCarriedDependence(void) const;

  void setLoopCarried(bool lc);

  void addSubEdge(DGEdge<SubT, SubT> *edge);

  void removeSubEdge(DGEdge<SubT, SubT> *edge);

  void removeSubEdges(void);

  virtual std::string toString(void) = 0;

  raw_ostream &print(raw_ostream &stream, std::string linePrefix = "");

  DependenceKind getKind(void) const;

  virtual ~DGEdge();

protected:
  DGEdge(DependenceKind k, DGNode<T> *src, DGNode<T> *dst);
  DGEdge(const DGEdge<T, SubT> &edgeToCopy);

private:
  DependenceKind kind;
  DGNode<T> *from;
  DGNode<T> *to;
  std::unordered_set<DGEdge<SubT, SubT> *> *subEdges;
  bool isLoopCarried;
};

template <class T, class SubT>
DGEdge<T, SubT>::DGEdge(DependenceKind k, DGNode<T> *src, DGNode<T> *dst)
  : kind{ k },
    from{ src },
    to{ dst },
    subEdges{ nullptr },
    isLoopCarried(false) {
  return;
}

template <class T, class SubT>
DGEdge<T, SubT>::DGEdge(const DGEdge<T, SubT> &edgeToCopy)
  : subEdges{ nullptr } {

  /*
   * Copy the vertices.
   */
  this->from = edgeToCopy.getSrcNode();
  this->to = edgeToCopy.getDstNode();

  /*
   * Copy the attributes.
   */
  this->setLoopCarried(edgeToCopy.isLoopCarriedDependence());
  this->kind = edgeToCopy.kind;

  /*
   * Copy the sub-edges.
   */
  if (edgeToCopy.subEdges != nullptr) {
    for (auto subEdge : (*edgeToCopy.subEdges)) {
      this->addSubEdge(subEdge);
    }
  }

  return;
}

template <class T, class SubT>
void DGEdge<T, SubT>::removeSubEdge(DGEdge<SubT, SubT> *edge) {

  /*
   * Remove the sub-edge
   */
  if (this->subEdges == nullptr) {
    abort();
  }
  this->subEdges->erase(edge);

  /*
   * Check if we can remove the set.
   */
  if (this->getNumberOfSubEdges() == 0) {
    this->removeSubEdges();
  }

  return;
}

template <class T, class SubT>
void DGEdge<T, SubT>::removeSubEdges(void) {
  if (this->subEdges == nullptr) {
    return;
  }

  this->subEdges->clear();
  delete this->subEdges;
  this->subEdges = nullptr;

  setLoopCarried(false);

  return;
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

  /*
   * Make sure there is a set allocated.
   */
  if (this->subEdges == nullptr) {
    this->subEdges = new std::unordered_set<DGEdge<SubT, SubT> *>();
  }
  assert(this->subEdges != nullptr);

  /*
   * Add the sub-edge.
   */
  this->subEdges->insert(edge);

  /*
   * Set the attributes.
   */
  isLoopCarried |= edge->isLoopCarriedDependence();

  return;
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
void DGEdge<T, SubT>::setSrcNode(DGNode<T> *f) {
  this->from = f;
}

template <class T, class SubT>
void DGEdge<T, SubT>::setDstNode(DGNode<T> *t) {
  this->to = t;
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
bool DGEdge<T, SubT>::isLoopCarriedDependence(void) const {
  return isLoopCarried;
}

template <class T, class SubT>
std::unordered_set<DGEdge<SubT, SubT> *> DGEdge<T, SubT>::getSubEdges(
    void) const {
  std::unordered_set<DGEdge<SubT, SubT> *> s;

  if (this->subEdges != nullptr) {
    for (auto subEdge : *(this->subEdges)) {
      s.insert(subEdge);
    }
  }

  return s;
}

template <class T, class SubT>
void DGEdge<T, SubT>::setLoopCarried(bool lc) {
  this->isLoopCarried = lc;
  return;
}

template <class T, class SubT>
uint64_t DGEdge<T, SubT>::getNumberOfSubEdges(void) const {
  if (this->subEdges == nullptr) {
    return 0;
  }

  return this->subEdges->size();
}

template <class T, class SubT>
typename DGEdge<T, SubT>::DependenceKind DGEdge<T, SubT>::getKind(void) const {
  return this->kind;
}

template <class T, class SubT>
DGEdge<T, SubT>::~DGEdge() {
  return;
}

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_DG_DGEDGE_H_
