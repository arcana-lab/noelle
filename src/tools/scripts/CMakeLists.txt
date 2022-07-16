# Install NOELLE's scripts
add_custom_target(
  NoelleToolScripts ALL
  installNOELLETools.sh
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
