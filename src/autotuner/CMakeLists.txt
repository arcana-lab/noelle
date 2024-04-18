set(AUTOTUNER_DESTINATION_DIR ${CMAKE_INSTALL_PREFIX}/autotuner)

install(
  DIRECTORY
    scripts
    src
    utils
  DESTINATION ${AUTOTUNER_DESTINATION_DIR}
)
# install(
#   DIRECTORY scripts
#   DESTINATION ${AUTOTUNER_DESTINATION_DIR}
# )
# install(
#   DIRECTORY src
#   DESTINATION ${AUTOTUNER_DESTINATION_DIR}
# )
# install(
#   DIRECTORY utils
#   DESTINATION ${AUTOTUNER_DESTINATION_DIR}
# )
install(
  FILES
    source-me-to-clean-python-virtual-environment
    source-me-to-setup-python-virtual-environment
  DESTINATION ${AUTOTUNER_DESTINATION_DIR}
)
