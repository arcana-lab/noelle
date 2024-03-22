noelle_component_declare(Noelle)

file(GLOB all_files "*")

set(directories "")

foreach(f IN LISTS all_files)
  if(IS_DIRECTORY ${f})
    list(APPEND directories ${f})
  endif()
endforeach()

foreach(module IN LISTS directories)
  include_directories(${module}/include)
  install(
    DIRECTORY ${module}/include
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    FILES_MATCHING PATTERN "*.hpp"
  )
  add_subdirectory(${module})
endforeach()
