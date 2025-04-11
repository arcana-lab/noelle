#pragma once

#include <string>

#include <arcana/noelle/core/Noelle.hpp>
#include <arcana/noelle/core/DependenceAnalysis.hpp>

namespace arcana::noelle {

enum DotOptions_value {
  DEFAULT = 0,
  ONLY_LC_EDGES = 1 << 0,
  HIDE_KNOWN_SCCS = 1 << 1,
  COLLAPSE_EDGES = 1 << 2,
  SHOW_CONTROL_DEPS = 1 << 3,
};

using DotOptions = unsigned int;

/*
 * The option `DA` allows for a custom dependence analysis to run on top of
 * noelle predefined analyses. Any dependence disproved by `DA` will be marked
 * with a different color (e.g. orange) in the file graph.
 *
 * If you intend to generate a graph where dependences dispoved by `DA` not
 * shown then provide a LoopContent `LC` computed with `DA` and pass `nullptr`
 * as `DA`. The resulting graph might me significatly different as SCC may now
 * split because of a missing edge.
 */
void exportToDotGraph(noelle::LoopContent *LC,
                      std::string outputFile,
                      DotOptions options = DEFAULT,
                      noelle::DependenceAnalysis *DA = nullptr);

} // namespace arcana::noelle
