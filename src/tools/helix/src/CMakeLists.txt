# Sources
set(Srcs 
  HELIX.cpp
  HELIX_applicabilityGuard.cpp
  HELIX_parallelization.cpp
  HELIX_lastIteration.cpp
  HELIX_prologue.cpp
  HELIX_synchronization.cpp
  HELIX_spiller.cpp
  HELIX_scheduler.cpp
  HELIX_inliner.cpp
  HELIX_dependences.cpp
  HELIX_stepper.cpp
  HELIX_sequentialSegments.cpp
  HELIX_sequentialSegment.cpp
  HELIX_linker.cpp
  HELIXTask.cpp
  SpilledLoopCarriedDependence.cpp
)

# Compilation flags
set_source_files_properties(${Srcs} PROPERTIES COMPILE_FLAGS " -std=c++17 -fPIC")

# Name of the LLVM pass
set(PassName "HELIX")

# configure LLVM 
find_package(LLVM 9 REQUIRED CONFIG)

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
include(AddLLVM)

message(STATUS "LLVM_DIR IS ${LLVM_CMAKE_DIR}.")

include_directories(${LLVM_INCLUDE_DIRS} 
  ../../heuristics/include 
  ../../parallelization_technique/include 
  ../../doall/include
  ../include 
  ./ 
  ${CMAKE_INSTALL_PREFIX}/include
  )

# Declare the LLVM pass to compile
add_llvm_library(${PassName} MODULE ${Srcs})
