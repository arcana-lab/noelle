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
#ifndef NOELLE_SRC_CORE_DG_DATA_DEPENDENCE_H_
#define NOELLE_SRC_CORE_DG_DATA_DEPENDENCE_H_

#include "noelle/core/DGEdge.hpp"

namespace arcana::noelle {

template <class T, class SubT>
class DataDependence : public DGEdge<T, SubT> {
  public:
    DataDependence(typename DGEdge<T, SubT>::DependenceKind k, DGNode<T> *src, DGNode<T> *dst);

    DataDependence(const DataDependence<T, SubT> &edgeToCopy);

    DataDependence() = delete;

    std::string toString(void) override ;

    static bool classof(const DGEdge<T, SubT> *s);
};

template <class T, class SubT>
DataDependence<T, SubT>::DataDependence(typename DGEdge<T, SubT>::DependenceKind k, DGNode<T> *src, DGNode<T> *dst)
  : DGEdge<T, SubT>(k, src, dst){
  return ;
}

template <class T, class SubT>
DataDependence<T, SubT>::DataDependence(const DataDependence<T, SubT> &edgeToCopy)
  : DGEdge<T, SubT>(edgeToCopy){
  return ;
}

template <class T, class SubT>
std::string DataDependence<T, SubT>::toString(void) {
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
  ros << "Data ";
  ros << this->dataDepToString();
  ros << (this->must ? " (must)" : " (may)");
  ros << (this->memory ? " from memory " : "");
  ros << "\n";
  ros.flush();
  return edgeStr;
}

template <class T, class SubT>
bool DataDependence<T, SubT>::classof(const DGEdge<T, SubT> *s) {
  auto sKind = s->getKind();
  return (sKind >= DGEdge<T, SubT>::DependenceKind::FIRST_DATA_DEPENDENCE) && (sKind <= DGEdge<T, SubT>::DependenceKind::LAST_DATA_DEPENDENCE);
}

}

#endif
