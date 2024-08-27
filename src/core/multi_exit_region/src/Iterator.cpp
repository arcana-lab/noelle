#include "arcana/noelle/core/MultiExitRegionTree.hpp"

namespace arcana::noelle {

template <typename It>
MultiExitRegionTree::Traversal<It>::Traversal(MultiExitRegionTree *T) : T(T) {}

template <typename It>
It MultiExitRegionTree::Traversal<It>::begin() {
  return It(T);
}

template <typename It>
It MultiExitRegionTree::Traversal<It>::end() {
  return It(nullptr);
}

MultiExitRegionTree::PreOrderIterator::PreOrderIterator(
    MultiExitRegionTree *T) {
  if (T == nullptr) {
    return;
  }

  if (T->isArtificialRoot) {
    for (auto C : T->children) {
      this->Ts.push(C);
    }
  } else {
    this->Ts.push(T);
  }
}

MultiExitRegionTree *MultiExitRegionTree::PreOrderIterator::operator*() {
  return this->Ts.front();
}

MultiExitRegionTree::PreOrderIterator &MultiExitRegionTree::PreOrderIterator::
operator++() {
  auto T = this->Ts.front();
  this->Ts.pop();
  for (auto C : T->children) {
    this->Ts.push(C);
  }
  return *this;
}

bool MultiExitRegionTree::PreOrderIterator::operator!=(
    MultiExitRegionTree::PreOrderIterator & /*other*/) {
  if (this->Ts.size() == 0) {
    return false;
  }
  return true;
}

} // namespace arcana::noelle
