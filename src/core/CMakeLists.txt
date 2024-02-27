noelle_component_declare(core)

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

noelle_component_commit(
  core # component name
  NoelleCore # shared library name
)

add_subdirectory(noelle_config)
