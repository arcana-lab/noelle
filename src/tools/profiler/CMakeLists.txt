noelle_tool_declare(Profiler)
target_sources(
  Profiler
  PRIVATE
  src/Pass.cpp
)

add_library(profiler SHARED
  src/Stopwatch.cpp
)

install(
  TARGETS profiler
  LIBRARY DESTINATION lib
)