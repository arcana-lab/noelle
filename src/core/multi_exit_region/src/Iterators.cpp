#include <stack>
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
    for (auto it = T->children.rbegin(); it != T->children.rend(); ++it) {
      this->Ts.push(*it);
    }
  } else {
    this->Ts.push(T);
  }
}

MultiExitRegionTree *MultiExitRegionTree::PreOrderIterator::operator*() {
  return this->Ts.top();
}

MultiExitRegionTree::PreOrderIterator &MultiExitRegionTree::PreOrderIterator::
operator++() {
  auto T = this->Ts.top();
  this->Ts.pop();
  for (auto it = T->children.rbegin(); it != T->children.rend(); ++it) {
    this->Ts.push(*it);
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
