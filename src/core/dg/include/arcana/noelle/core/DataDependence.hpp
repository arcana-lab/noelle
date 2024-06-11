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

#include "arcana/noelle/core/DGEdge.hpp"

namespace arcana::noelle {

enum DataDependenceType { DG_DATA_RAW, DG_DATA_WAR, DG_DATA_WAW };

template <class T, class SubT>
class DataDependence : public DGEdge<T, SubT> {
public:
  DataDependence() = delete;

  DataDependenceType getDataDependenceType(void) const;

  bool isRAWDependence() const {
    return dataDepType == DG_DATA_RAW;
  }

  bool isWARDependence() const {
    return dataDepType == DG_DATA_WAR;
  }

  bool isWAWDependence() const {
    return dataDepType == DG_DATA_WAW;
  }

  std::string dataDepToString(void);

  static bool classof(const DGEdge<T, SubT> *s);

  static DataDependenceType stringToDataDep(std::string &str);

protected:
  DataDependence(typename DGEdge<T, SubT>::DependenceKind k,
                 DGNode<T> *src,
                 DGNode<T> *dst,
                 DataDependenceType t);
  DataDependence(const DataDependence<T, SubT> &edgeToCopy);

private:
  DataDependenceType dataDepType;
};

template <class T, class SubT>
DataDependence<T, SubT>::DataDependence(
    typename DGEdge<T, SubT>::DependenceKind k,
    DGNode<T> *src,
    DGNode<T> *dst,
    DataDependenceType t)
  : DGEdge<T, SubT>(k, src, dst),
    dataDepType{ t } {
  return;
}

template <class T, class SubT>
DataDependence<T, SubT>::DataDependence(
    const DataDependence<T, SubT> &edgeToCopy)
  : DGEdge<T, SubT>(edgeToCopy) {
  this->dataDepType = edgeToCopy.getDataDependenceType();
  return;
}

template <class T, class SubT>
DataDependenceType DataDependence<T, SubT>::getDataDependenceType(void) const {
  return this->dataDepType;
}

template <class T, class SubT>
bool DataDependence<T, SubT>::classof(const DGEdge<T, SubT> *s) {
  auto sKind = s->getKind();
  return (sKind >= DGEdge<T, SubT>::DependenceKind::FIRST_DATA_DEPENDENCE)
         && (sKind <= DGEdge<T, SubT>::DependenceKind::LAST_DATA_DEPENDENCE);
}

template <class T, class SubT>
std::string DataDependence<T, SubT>::dataDepToString(void) {
  if (this->isRAWDependence()) {
    return "RAW";
  } else if (this->isWARDependence()) {
    return "WAR";
  } else if (this->isWAWDependence()) {
    return "WAW";
  }
  abort();
}

template <class T, class SubT>
DataDependenceType DataDependence<T, SubT>::stringToDataDep(std::string &str) {
  if (str == "RAW") {
    return DG_DATA_RAW;
  } else if (str == "WAR") {
    return DG_DATA_WAR;
  } else if (str == "WAW") {
    return DG_DATA_WAW;
  }
  abort();
}

} // namespace arcana::noelle

#endif
