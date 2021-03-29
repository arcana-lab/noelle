#pragma once

#include "Annotation.hpp"

namespace AutoMP
{
  AnnotationSet parseAnnotationsForInst(const llvm::Instruction *i);
};
