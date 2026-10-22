if(NOT DEFINED AXIOM_BUILD_DIR OR NOT DEFINED AXIOM_STAGE_DIR OR
   NOT DEFINED CONSUMER_SOURCE_DIR OR NOT DEFINED CONSUMER_BUILD_DIR)
  message(FATAL_ERROR "kernel-sdk consumer test requires build, stage, source, and consumer build directories")
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

file(REMOVE_RECURSE "${AXIOM_STAGE_DIR}" "${CONSUMER_BUILD_DIR}")

run_checked("staged install"
  "${CMAKE_COMMAND}" --install "${AXIOM_BUILD_DIR}" --prefix "${AXIOM_STAGE_DIR}")
run_checked("kernel-sdk consumer configure"
  "${CMAKE_COMMAND}" -S "${CONSUMER_SOURCE_DIR}" -B "${CONSUMER_BUILD_DIR}"
  "-DCMAKE_PREFIX_PATH=${AXIOM_STAGE_DIR}")
run_checked("kernel-sdk consumer build"
  "${CMAKE_COMMAND}" --build "${CONSUMER_BUILD_DIR}")
run_checked("kernel-sdk consumer run"
  "${CONSUMER_BUILD_DIR}/axiom_lhme_kernel_sdk_consumer")
