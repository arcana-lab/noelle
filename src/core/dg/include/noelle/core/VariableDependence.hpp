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
#ifndef NOELLE_SRC_CORE_DG_VARIABLE_DEPENDENCE_H_
#define NOELLE_SRC_CORE_DG_VARIABLE_DEPENDENCE_H_

#include "noelle/core/DataDependence.hpp"

namespace arcana::noelle {

template <class T, class SubT>
class VariableDependence : public DataDependence<T, SubT> {
public:
  VariableDependence(DGNode<T> *src, DGNode<T> *dst);

  VariableDependence(const VariableDependence<T, SubT> &edgeToCopy);

  VariableDependence() = delete;

  std::string toString(void) override;

  static bool classof(const DGEdge<T, SubT> *s);
};

template <class T, class SubT>
VariableDependence<T, SubT>::VariableDependence(DGNode<T> *src, DGNode<T> *dst)
  : DataDependence<T, SubT>(
      DGEdge<T, SubT>::DependenceKind::VARIABLE_DEPENDENCE,
      src,
      dst) {
  return;
}

template <class T, class SubT>
VariableDependence<T, SubT>::VariableDependence(
    const VariableDependence<T, SubT> &edgeToCopy)
  : DataDependence<T, SubT>(edgeToCopy) {
  return;
}

template <class T, class SubT>
std::string VariableDependence<T, SubT>::toString(void) {
  if (this->getNumberOfSubEdges() > 0) {
    std::string edgesStr;
    raw_string_ostream ros(edgesStr);
    for (auto edge : this->getSubEdges())
      ros << edge->toString();
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
  ros << "\n";
  ros.flush();
  return edgeStr;
}

template <class T, class SubT>
bool VariableDependence<T, SubT>::classof(const DGEdge<T, SubT> *s) {
  auto sKind = s->getKind();
  return (sKind == DGEdge<T, SubT>::DependenceKind::VARIABLE_DEPENDENCE);
}

} // namespace arcana::noelle

#endif
