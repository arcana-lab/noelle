add_noelle_component(tools)

foreach(module IN LISTS NOELLE_CORE_MODULES)
  include_directories(../core/${module}/include)
endforeach()

foreach(module IN LISTS NOELLE_TOOL_MODULES)
  include_directories(${module}/include)
  install(
    DIRECTORY ${module}/include
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    FILES_MATCHING PATTERN "*.hpp"
  )
endforeach()

foreach(module IN LISTS NOELLE_TOOL_MODULES)
  add_subdirectory(${module})
endforeach()
add_subdirectory(scripts)

get_target_property(tools_SOURCES tools SOURCES)
add_llvm_library(NoelleTools MODULE "${tools_SOURCES}")
