/*
 * Copyright 2023 - 2024  Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_DG_MUST_MEMORY_DEPENDENCE_H_
#define NOELLE_SRC_CORE_DG_MUST_MEMORY_DEPENDENCE_H_

#include "arcana/noelle/core/MemoryDependence.hpp"

namespace arcana::noelle {

template <class T, class SubT>
class MustMemoryDependence : public MemoryDependence<T, SubT> {
public:
  MustMemoryDependence(DGNode<T> *src, DGNode<T> *dst, DataDependenceType t);

  MustMemoryDependence(const MustMemoryDependence<T, SubT> &edgeToCopy);

  MustMemoryDependence() = delete;

  std::string toString(void) override;

  static bool classof(const DGEdge<T, SubT> *s);
};

template <class T, class SubT>
MustMemoryDependence<T, SubT>::MustMemoryDependence(DGNode<T> *src,
                                                    DGNode<T> *dst,
                                                    DataDependenceType t)
  : MemoryDependence<T, SubT>(
      DGEdge<T, SubT>::DependenceKind::MUST_MEMORY_DEPENDENCE,
      src,
      dst,
      t) {
  return;
}

template <class T, class SubT>
MustMemoryDependence<T, SubT>::MustMemoryDependence(
    const MustMemoryDependence<T, SubT> &edgeToCopy)
  : MemoryDependence<T, SubT>(edgeToCopy) {
  return;
}

template <class T, class SubT>
std::string MustMemoryDependence<T, SubT>::toString(void) {
  if (this->getNumberOfSubEdges() > 0) {
    std::string edgesStr;
    raw_string_ostream ros(edgesStr);
    for (auto edge : this->getSubEdges()) {
      ros << edge->toString();
    }
    return ros.str();
  }
  std::string edgeStr;
  raw_string_ostream ros(edgeStr);
  ros << "Attributes: ";
  if (this->isLoopCarriedDependence()) {
    ros << "Loop-carried ";
  }
  ros << "Data ";
  ros << this->dataDepToString();
  ros << " (must) from memory\n";
  ros.flush();
  return edgeStr;
}

template <class T, class SubT>
bool MustMemoryDependence<T, SubT>::classof(const DGEdge<T, SubT> *s) {
  auto sKind = s->getKind();
  return (sKind == DGEdge<T, SubT>::DependenceKind::MUST_MEMORY_DEPENDENCE);
}

} // namespace arcana::noelle

#endif
