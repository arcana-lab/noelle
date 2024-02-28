target_sources(
  Noelle # component name
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Hot_BasicBlock.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Hot.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Hot_Function.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Hot_Instruction.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Hot_Loop.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Hot_Module.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/HotProfiler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Hot_SCC.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Pass.cpp
)
