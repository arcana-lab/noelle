# making all core header files available

file(GLOB all_core_files "../core/*")
set(directories "")

foreach(f IN LISTS all_core_files)
  if(IS_DIRECTORY ${f})
    include_directories(${f}/include)
  endif()
endforeach()

# adding source for Tools

file(GLOB all_tool_files "*")
set(directories "")

foreach(f IN LISTS all_tool_files)
  if(IS_DIRECTORY ${f})
    list(APPEND directories ${f})
  endif()
endforeach()

if(NOELLE_REPL STREQUAL OFF)
  list(REMOVE_ITEM directories ${CMAKE_CURRENT_SOURCE_DIR}/repl)
endif()

foreach(module IN LISTS directories)
  include_directories(${module}/include)
  install(
    DIRECTORY ${module}/include
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    FILES_MATCHING PATTERN "*.hpp"
  )
endforeach()

foreach(module IN LISTS directories)
  add_subdirectory(${module})
endforeach()
