include(AddLLVM)
include(HandleLLVMOptions)
set(LLVM_ENABLE_UNWIND_TABLES ON)

add_subdirectory(core)

if(NOELLE_TOOLS STREQUAL ON)
  add_subdirectory(tools)
endif()

if(NOELLE_AUTOTUNER STREQUAL ON)
  add_subdirectory(autotuner)
endif()
