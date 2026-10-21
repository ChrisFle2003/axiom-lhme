if(NOT DEFINED BRUCE_BUILD_DIR OR NOT DEFINED BRUCE_STAGE_DIR OR
   NOT DEFINED CONSUMER_SOURCE_DIR OR NOT DEFINED CONSUMER_BUILD_DIR)
  message(FATAL_ERROR "external-consumer test requires build, stage, source, and consumer build directories")
endif()

function(run_checked step)
  execute_process(
    COMMAND ${ARGN}
    RESULT_VARIABLE command_result
    OUTPUT_VARIABLE command_stdout
    ERROR_VARIABLE command_stderr)
  if(NOT command_result EQUAL 0)
    message(FATAL_ERROR
      "${step} failed with code ${command_result}\nstdout:\n${command_stdout}\nstderr:\n${command_stderr}")
  endif()
endfunction()

file(REMOVE_RECURSE "${BRUCE_STAGE_DIR}" "${CONSUMER_BUILD_DIR}")

run_checked("staged install"
  "${CMAKE_COMMAND}" --install "${BRUCE_BUILD_DIR}" --prefix "${BRUCE_STAGE_DIR}")
run_checked("external consumer configure"
  "${CMAKE_COMMAND}" -S "${CONSUMER_SOURCE_DIR}" -B "${CONSUMER_BUILD_DIR}"
  "-DCMAKE_PREFIX_PATH=${BRUCE_STAGE_DIR}")
run_checked("external consumer build"
  "${CMAKE_COMMAND}" --build "${CONSUMER_BUILD_DIR}")
run_checked("external consumer run"
  "${CONSUMER_BUILD_DIR}/bruce_lhme_external_consumer")
