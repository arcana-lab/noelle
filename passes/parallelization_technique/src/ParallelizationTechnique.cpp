#include "ParallelizationTechnique.hpp"

using namespace llvm;

ParallelizationTechnique::ParallelizationTechnique (Module &module, Verbosity v)
  :
  module{module},
  verbose{v}
  {

  return ;
}
