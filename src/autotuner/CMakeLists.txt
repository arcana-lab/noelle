set(AUTOTUNER_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/autotuner)

install(
  DIRECTORY
    scripts
    src
    utils
  DESTINATION ${AUTOTUNER_INSTALL_DIR}
)

install(
  PROGRAMS
    scripts/compile
    scripts/run
  DESTINATION ${AUTOTUNER_INSTALL_DIR}/scripts
)

install(
  FILES
    source-me-to-clean-python-virtual-environment
    source-me-to-setup-python-virtual-environment
  DESTINATION ${AUTOTUNER_INSTALL_DIR}
)
