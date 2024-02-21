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

add_subdirectory(noelle_config)
add_subdirectory(scripts)
