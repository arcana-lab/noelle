#include "arcana/noelle/core/MultiExitRegionTree.hpp"

namespace arcana::noelle {

MultiExitRegionTree::iterator::iterator(MultiExitRegionTree *T) {
  if (T == nullptr) {
    return;
  }

  if (T->isArtificialRoot) {
    for (auto C : T->getChildren()) {
      this->Ts.push(C);
    }
  } else {
    this->Ts.push(T);
  }
}

MultiExitRegionTree *MultiExitRegionTree::iterator::operator*() {
  return this->Ts.front();
}

MultiExitRegionTree::iterator &MultiExitRegionTree::iterator::operator++() {
  auto T = this->Ts.front();
  this->Ts.pop();
  for (auto C : T->getChildren()) {
    this->Ts.push(C);
  }
  return *this;
}

bool MultiExitRegionTree::iterator::operator!=(
    MultiExitRegionTree::iterator & /*other*/) {
  if (this->Ts.size() == 0) {
    return false;
  }
  return true;
}

} // namespace arcana::noelle
