# Install NOELLE's scripts
add_custom_target(
  NoelleScripts ALL
  installNOELLE.sh
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
