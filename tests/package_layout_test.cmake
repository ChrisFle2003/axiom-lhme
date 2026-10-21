if(NOT DEFINED BRUCE_BUILD_DIR OR NOT DEFINED BRUCE_STAGE_DIR OR
   NOT DEFINED BRUCE_INSTALL_LIBDIR)
  message(FATAL_ERROR "package-layout test requires build, stage, and library directories")
endif()

file(REMOVE_RECURSE "${BRUCE_STAGE_DIR}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${BRUCE_BUILD_DIR}" --prefix "${BRUCE_STAGE_DIR}"
  RESULT_VARIABLE install_result
  OUTPUT_VARIABLE install_stdout
  ERROR_VARIABLE install_stderr)

if(NOT install_result EQUAL 0)
  message(FATAL_ERROR
    "staged install failed with code ${install_result}\nstdout:\n${install_stdout}\nstderr:\n${install_stderr}")
endif()

set(package_directory
    "${BRUCE_STAGE_DIR}/${BRUCE_INSTALL_LIBDIR}/cmake/bruce_lhme")
foreach(package_file IN ITEMS
    bruce_lhmeConfig.cmake
    bruce_lhmeConfigVersion.cmake
    bruce_lhmeTargets.cmake)
  if(NOT EXISTS "${package_directory}/${package_file}")
    message(FATAL_ERROR
      "missing installed package file: ${package_directory}/${package_file}\ninstall stdout:\n${install_stdout}\ninstall stderr:\n${install_stderr}")
  endif()
endforeach()
