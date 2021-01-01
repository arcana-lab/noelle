#pragma once

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Instructions.h"
#include <string>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace AutoMP
{

  class Annotation
  {
  public:
    Annotation() : Annotation(nullptr, "", "") { }
    Annotation(llvm::Loop *l, std::string k, std::string v) : loop(l), key(k), value(v) { }

    std::string getKey() const { return key; }
    std::string getValue() const { return value; }
    void setLoop(llvm::Loop *l) { loop = l; }
    llvm::Loop *getLoop() const { return loop; }

    // comparison operators that are needed by some stdlib things like unordered_map, unordered_set, etc
    bool operator<(const Annotation &a) const { return true; } // don't care about order
    bool operator==(const Annotation &a) const { return !key.compare(a.key) && !value.compare(a.value) && a.getLoop() == loop; }

    // printing stuff
    // NOTE: Could have made this template but that seems weird...
    friend std::ostream &operator<<(std::ostream &, const Annotation &) { assert(false && "Not implemented"); }
    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &, const Annotation &);

  private:
    llvm::Loop *loop;
    std::string key;
    std::string value;
  };

  // Could have done the custom specialization of std::hash as well, described in the example at
  // https://en.cppreference.com/w/cpp/utility/hash
  class AnnotHashFn
  {
  public:
    size_t operator()(const Annotation &a) const
    {
      using namespace std;
      return hash<string>()( a.getKey() )
        ^ ((hash<string>()( a.getValue() ) << 1) >> 1)
        ^ ((hash<llvm::Loop *>()( a.getLoop())) << 1);
    }
  };

  // store annotations in these things
  typedef std::unordered_set<Annotation, AnnotHashFn> AnnotationSet;
  typedef std::unordered_map<llvm::Loop *, Annotation> LoopToAnnotationMap;

  // return true if matches an annotation annotation set
  bool withinAnnotationSet(const AnnotationSet &, std::string, std::string, const llvm::Loop *);

  // Used for debugging purposes
  llvm::raw_ostream &operator<<(llvm::raw_ostream &, const std::pair<const llvm::Instruction *, const AnnotationSet &> &);

  /******************************************************************************
   * XXX BELOW THIS IS NOT WORKING XXX
   */
  /*
   * Syntax for reductions:
   *    #pragma note noelle reduction = <type>:<variable1>,<variable2>,...
   *
   * Limitations:
   * Doesn't support user-defined reductions yet (probably never will)
   */
  class ReduxAnnotation : public Annotation
  {
    enum class Type
    {
      Sum,
      Product
    };

  public:
    ReduxAnnotation();

  private:
    Type type;
    llvm::Value *redux_var; // is Value specific enough?
    std::set<llvm::Value *> associated_vars; // better naming later
  };

  class PrivateAnnotation : public Annotation
  {

  };

} // namespace AutoMP
