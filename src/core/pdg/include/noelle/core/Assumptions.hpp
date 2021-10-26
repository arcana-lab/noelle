#ifndef LLVM_LIBERTY_ASSUMPTIONS_H
#define LLVM_LIBERTY_ASSUMPTIONS_H

#include "llvm/Pass.h"
#include "llvm/IR/CallSite.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"

#include <set>
#include <unordered_set>
#include <memory>

using namespace llvm;

namespace llvm {
  namespace noelle {

    template <class T> class DGEdge;

    // Criticism is a PDG edge with a boolean value to differentiate loop-carried
    // from intra-iteration edges. Also specify type of dep (mem/reg/ctrl)
    typedef DGEdge<Value> Criticism;

    typedef std::set<Criticism *> Criticisms;

    class Remedy;
    typedef std::shared_ptr<Remedy> Remedy_ptr;

    struct RemedyCompare;

    typedef std::set<Remedy_ptr, RemedyCompare> Remedies;

    class Remedy {
    public:
      Criticisms resolvedC;
      unsigned long cost;

      //Loop *loop;

      //virtual void apply(Task *task) = 0;
      virtual bool compare(const Remedy_ptr rhs) const = 0;
      virtual StringRef getRemedyName() const = 0;

      virtual bool hasSubRemedies() { return false; }
      virtual Remedies *getSubRemedies() { return nullptr; }

      virtual bool isExpensive() { return false; }
    };

    typedef std::shared_ptr<Remedies> Remedies_ptr;

    struct RemedyCompare {
      bool operator()(const Remedy_ptr &lhs, const Remedy_ptr &rhs) const {
        // if same remedy type use custom comparator,
        // otherwise compare based on cost or remedy name (if cost the same)
        if (lhs->getRemedyName().equals(rhs->getRemedyName()))
          return lhs->compare(rhs);
        else if (lhs->cost == rhs->cost)
          return (lhs->getRemedyName().compare(rhs->getRemedyName()) == -1);
        else
          return lhs->cost < rhs->cost;
      }
    };

    struct RemediesCompare {
      bool operator()(const Remedies_ptr &lhs, const Remedies_ptr &rhs) const {

        RemedyCompare remedyCompare;

        // compute total costs
        unsigned costLHS = 0;
        for (auto r : *lhs) {
          costLHS += r->cost;
        }
        unsigned costRHS = 0;
        for (auto r : *rhs) {
          costRHS += r->cost;
        }

        // if different sizes compare their costs
        if (lhs->size() != rhs->size()) {
          return costLHS < costRHS;
        }

        auto itLHS = lhs->begin();
        auto itRHS = rhs->begin();
        bool identical = true;
        while (itLHS != lhs->end()) {
          // check if each remedy is equal
          if (remedyCompare(*itLHS, *itRHS) || remedyCompare(*itRHS, *itLHS)) {
            identical = false;
            break;
          }
          ++itLHS;
          ++itRHS;
        }
        if (identical)
          return false;

        return costLHS < costRHS;
      }
    };

    typedef std::set<Remedies_ptr, RemediesCompare> SetOfRemedies;
    typedef std::unique_ptr<SetOfRemedies> SetOfRemedies_ptr;

  }
}

#endif
