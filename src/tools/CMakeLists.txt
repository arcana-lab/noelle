noelle_component_declare(tools)

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

noelle_component_commit(
  tools # component name
  NoelleTools # shared library name
)
