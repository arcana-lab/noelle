add_noelle_component(core)

foreach(module IN LISTS NOELLE_CORE_MODULES)
  include_directories(${module}/include)
  install(
    DIRECTORY ${module}/include
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    FILES_MATCHING PATTERN "*.hpp"
  )
endforeach()

foreach(module IN LISTS NOELLE_CORE_MODULES)
  add_subdirectory(${module})
endforeach()

get_target_property(core_SOURCES core SOURCES)
add_llvm_library(NoelleCore MODULE "${core_SOURCES}")

add_subdirectory(noelle_config)
add_subdirectory(scripts)
