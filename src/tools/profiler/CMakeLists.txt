noelle_tool_declare(Profiler)
target_sources(
  Profiler
  PRIVATE
  src/Pass.cpp
)

add_library(stopwatch SHARED
  src/Stopwatch.cpp
)