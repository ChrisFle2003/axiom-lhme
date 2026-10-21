cmake_minimum_required(VERSION 3.25)

project(bruce_lhme VERSION 0.1.0 LANGUAGES C CXX)

find_package(Threads REQUIRED)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(BRUCE_SANITIZER "none" CACHE STRING
    "Runtime sanitizer: none, address, thread, or undefined")
set_property(CACHE BRUCE_SANITIZER PROPERTY STRINGS none address thread undefined)
set(BRUCE_SANITIZER_LINK_OPTION "")
if(NOT BRUCE_SANITIZER STREQUAL "none")
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "BRUCE_SANITIZER requires a GNU or Clang compiler")
  endif()
  if(BRUCE_SANITIZER STREQUAL "address")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
    set(BRUCE_SANITIZER_LINK_OPTION -fsanitize=address)
  elseif(BRUCE_SANITIZER STREQUAL "thread")
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
    set(BRUCE_SANITIZER_LINK_OPTION -fsanitize=thread)
  elseif(BRUCE_SANITIZER STREQUAL "undefined")
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
    set(BRUCE_SANITIZER_LINK_OPTION -fsanitize=undefined)
  else()
    message(FATAL_ERROR "BRUCE_SANITIZER must be none, address, thread, or undefined")
  endif()
endif()

include(CTest)

add_library(bruce_lhme_core STATIC
  src/gguf/reader.cpp
  src/gguf/pattern_scanner.cpp
  src/gguf/report.cpp
  src/gguf/storage_planner.cpp
  src/core/formula.cpp
  src/core/fibonacci_kernel.cpp
  src/core/power_offset_kernel.cpp
  src/core/trace.cpp
  src/kevin/grid.cpp
  src/kevin/domain_projection.cpp
  src/kevin/filter.cpp
  src/kevin/cpu_executor.cpp
  src/kevin/memory_store.cpp
  src/kevin/snapshot.cpp
  src/cpu/scheduler.cpp
  src/cpu/streaming.cpp
  src/cache/result_cache.cpp
  src/engine.cpp
  src/formula_override_store.cpp
  src/formula_override_snapshot.cpp
  src/storage_policy.cpp
  src/c_api.cpp
)
target_include_directories(bruce_lhme_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(bruce_lhme_core PUBLIC cxx_std_20)
target_link_libraries(bruce_lhme_core PUBLIC Threads::Threads)
if(BRUCE_SANITIZER_LINK_OPTION)
  # A static archive contains instrumented objects. Export the matching runtime
  # link option so find_package() consumers link successfully as well.
  target_link_options(bruce_lhme_core INTERFACE ${BRUCE_SANITIZER_LINK_OPTION})
endif()
set_target_properties(bruce_lhme_core PROPERTIES EXPORT_NAME core)

add_executable(core_formula_tests
  tests/support/test_main.cpp
  tests/core_formula_tests.cpp
)
target_include_directories(core_formula_tests PRIVATE tests/support include)
target_link_libraries(core_formula_tests PRIVATE bruce_lhme_core)
add_test(NAME core_formula_tests COMMAND core_formula_tests)

add_executable(gguf_reader_tests
  tests/support/test_main.cpp
  tests/gguf_reader_tests.cpp
)
target_include_directories(gguf_reader_tests PRIVATE tests/support include)
target_link_libraries(gguf_reader_tests PRIVATE bruce_lhme_core)
add_test(NAME gguf_reader_tests COMMAND gguf_reader_tests)

add_executable(gguf_storage_plan_tests
  tests/support/test_main.cpp
  tests/gguf_storage_plan_tests.cpp
)
target_include_directories(gguf_storage_plan_tests PRIVATE tests/support include)
target_link_libraries(gguf_storage_plan_tests PRIVATE bruce_lhme_core)
add_test(NAME gguf_storage_plan_tests COMMAND gguf_storage_plan_tests)

add_executable(gguf_pattern_scan_tests
  tests/support/test_main.cpp
  tests/gguf_pattern_scan_tests.cpp
)
target_include_directories(gguf_pattern_scan_tests PRIVATE tests/support include)
target_link_libraries(gguf_pattern_scan_tests PRIVATE bruce_lhme_core)
add_test(NAME gguf_pattern_scan_tests COMMAND gguf_pattern_scan_tests)

add_executable(gguf_report_tests
  tests/support/test_main.cpp
  tests/gguf_report_tests.cpp
)
target_include_directories(gguf_report_tests PRIVATE
  tests/support include tools/gguf_inspect
)
target_link_libraries(gguf_report_tests PRIVATE bruce_lhme_core)
target_sources(gguf_report_tests PRIVATE tools/gguf_inspect/command.cpp)
add_test(NAME gguf_report_tests COMMAND gguf_report_tests)

add_executable(bruce_gguf_inspect
  tools/gguf_inspect/command.cpp
  tools/gguf_inspect/main.cpp
)
target_include_directories(bruce_gguf_inspect PRIVATE include tools/gguf_inspect)
target_link_libraries(bruce_gguf_inspect PRIVATE bruce_lhme_core)

add_executable(core_budget_trace_tests
  tests/support/test_main.cpp
  tests/core_budget_trace_tests.cpp
)
target_include_directories(core_budget_trace_tests PRIVATE tests/support include)
target_link_libraries(core_budget_trace_tests PRIVATE bruce_lhme_core)
add_test(NAME core_budget_trace_tests COMMAND core_budget_trace_tests)

add_executable(cpu_scheduler_tests
  tests/support/test_main.cpp
  tests/cpu_scheduler_tests.cpp
)
target_include_directories(cpu_scheduler_tests PRIVATE tests/support include)
target_link_libraries(cpu_scheduler_tests PRIVATE bruce_lhme_core)
add_test(NAME cpu_scheduler_tests COMMAND cpu_scheduler_tests)

add_executable(streaming_tests
  tests/support/test_main.cpp
  tests/streaming_tests.cpp
)
target_include_directories(streaming_tests PRIVATE tests/support include)
target_link_libraries(streaming_tests PRIVATE bruce_lhme_core)
add_test(NAME streaming_tests COMMAND streaming_tests)

add_executable(cache_tests
  tests/support/test_main.cpp
  tests/cache_tests.cpp
)
target_include_directories(cache_tests PRIVATE tests/support include)
target_link_libraries(cache_tests PRIVATE bruce_lhme_core)
add_test(NAME cache_tests COMMAND cache_tests)

add_executable(engine_tests
  tests/support/test_main.cpp
  tests/engine_tests.cpp
)
target_include_directories(engine_tests PRIVATE tests/support include)
target_link_libraries(engine_tests PRIVATE bruce_lhme_core)
add_test(NAME engine_tests COMMAND engine_tests)

add_executable(c_api_tests
  tests/support/test_main.cpp
  tests/c_api_tests.cpp
)
target_include_directories(c_api_tests PRIVATE tests/support include)
target_link_libraries(c_api_tests PRIVATE bruce_lhme_core)
add_test(NAME c_api_tests COMMAND c_api_tests)

add_executable(c_api_header_smoke tests/c_api_header_smoke.c)
target_include_directories(c_api_header_smoke PRIVATE include)
target_link_libraries(c_api_header_smoke PRIVATE bruce_lhme_core)
set_target_properties(c_api_header_smoke PROPERTIES LINKER_LANGUAGE CXX)
add_test(NAME c_api_header_smoke COMMAND c_api_header_smoke)

add_executable(lhme_formula_demo demos/lhme/formula_demo.cpp)
target_include_directories(lhme_formula_demo PRIVATE include demos/lhme/support)
target_link_libraries(lhme_formula_demo PRIVATE bruce_lhme_core)
add_test(NAME lhme_formula_demo COMMAND lhme_formula_demo)

add_executable(lhme_range_demo demos/lhme/range_demo.cpp)
target_include_directories(lhme_range_demo PRIVATE include demos/lhme/support)
target_link_libraries(lhme_range_demo PRIVATE bruce_lhme_core)
add_test(NAME lhme_range_demo COMMAND lhme_range_demo)

add_executable(lhme_memory_demo demos/lhme/memory_demo.cpp)
target_include_directories(lhme_memory_demo PRIVATE include demos/lhme/support)
target_link_libraries(lhme_memory_demo PRIVATE bruce_lhme_core)
add_test(NAME lhme_memory_demo COMMAND lhme_memory_demo)

add_executable(lhme_streaming_benchmark
  demos/lhme/benchmarks/streaming_benchmark.cpp
)
target_include_directories(lhme_streaming_benchmark PRIVATE include demos/lhme/support)
target_link_libraries(lhme_streaming_benchmark PRIVATE bruce_lhme_core)
add_test(NAME lhme_streaming_benchmark COMMAND lhme_streaming_benchmark)

add_executable(lhme_controlled_stress
  demos/lhme/benchmarks/stress/controlled_stress.cpp
)
target_include_directories(lhme_controlled_stress PRIVATE include demos/lhme/support)
target_link_libraries(lhme_controlled_stress PRIVATE bruce_lhme_core)

add_executable(isolated_protocol_tests
  tests/support/test_main.cpp
  tests/isolated_protocol_tests.cpp
)
target_include_directories(isolated_protocol_tests PRIVATE
  tests/support
  include
  demos/lhme/benchmarks/stress
)
target_link_libraries(isolated_protocol_tests PRIVATE bruce_lhme_core)
add_test(NAME isolated_protocol_tests COMMAND isolated_protocol_tests)

add_executable(isolated_runner_tests
  tests/support/test_main.cpp
  tests/isolated_runner_tests.cpp
)
add_executable(isolated_runner_fixture tests/isolated_runner_fixture.cpp)
target_include_directories(isolated_runner_fixture PRIVATE
  include
  demos/lhme/benchmarks/stress
)
target_include_directories(isolated_runner_tests PRIVATE
  tests/support
  include
  demos/lhme/benchmarks/stress
)
target_compile_definitions(isolated_runner_tests PRIVATE
  LHME_STRESS_EXECUTABLE="$<TARGET_FILE:lhme_controlled_stress>"
  LHME_ISOLATED_FIXTURE_EXECUTABLE="$<TARGET_FILE:isolated_runner_fixture>"
)
target_link_libraries(isolated_runner_tests PRIVATE bruce_lhme_core)
add_dependencies(isolated_runner_tests lhme_controlled_stress isolated_runner_fixture)
add_test(NAME isolated_runner_tests COMMAND isolated_runner_tests)

add_executable(table_storage_tests
  tests/support/test_main.cpp
  tests/table_storage_tests.cpp
)
target_include_directories(table_storage_tests PRIVATE
  tests/support
  include
  demos/lhme/benchmarks/table_memory
)
target_link_libraries(table_storage_tests PRIVATE bruce_lhme_core)
add_test(NAME table_storage_tests COMMAND table_storage_tests)

add_executable(fair_benchmark_contract_tests
  tests/support/test_main.cpp
  tests/fair_benchmark_contract_tests.cpp
)
target_include_directories(fair_benchmark_contract_tests PRIVATE
  tests/support
  include
  demos/lhme/benchmarks/fair
)
target_link_libraries(fair_benchmark_contract_tests PRIVATE bruce_lhme_core)
add_test(NAME fair_benchmark_contract_tests COMMAND fair_benchmark_contract_tests)

add_executable(lhme_fair_storage_benchmark
  demos/lhme/benchmarks/fair/fair_storage_benchmark.cpp
)
target_include_directories(lhme_fair_storage_benchmark PRIVATE
  include demos/lhme/support demos/lhme/benchmarks/fair demos/lhme/benchmarks/stress
)
target_link_libraries(lhme_fair_storage_benchmark PRIVATE bruce_lhme_core)
add_test(NAME lhme_fair_storage_benchmark COMMAND lhme_fair_storage_benchmark --quick)

add_test(
  NAME fair_benchmark_docs_test
  COMMAND ${CMAKE_COMMAND}
    -DREVIEW_FILE=${CMAKE_CURRENT_SOURCE_DIR}/docs/benchmark-review-2026-10-21.md
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/fair_benchmark_docs_test.cmake
)

add_executable(storage_policy_tests
  tests/support/test_main.cpp
  tests/storage_policy_tests.cpp
)
target_include_directories(storage_policy_tests PRIVATE tests/support include)
target_link_libraries(storage_policy_tests PRIVATE bruce_lhme_core)
add_test(NAME storage_policy_tests COMMAND storage_policy_tests)

add_executable(formula_override_store_tests
  tests/support/test_main.cpp
  tests/formula_override_store_tests.cpp
)
target_include_directories(formula_override_store_tests PRIVATE tests/support include)
target_link_libraries(formula_override_store_tests PRIVATE bruce_lhme_core)
add_test(NAME formula_override_store_tests COMMAND formula_override_store_tests)

add_executable(formula_override_snapshot_tests
  tests/support/test_main.cpp
  tests/formula_override_snapshot_tests.cpp
)
target_include_directories(formula_override_snapshot_tests PRIVATE tests/support include)
target_link_libraries(formula_override_snapshot_tests PRIVATE bruce_lhme_core)
add_test(NAME formula_override_snapshot_tests COMMAND formula_override_snapshot_tests)

add_executable(technique_tests
  tests/support/test_main.cpp
  tests/technique_tests.cpp
)
target_include_directories(technique_tests PRIVATE tests/support include)
target_link_libraries(technique_tests PRIVATE bruce_lhme_core)
add_test(NAME technique_tests COMMAND technique_tests)

add_executable(kernel_sdk_contract_tests
  tests/support/test_main.cpp
  tests/kernel_sdk_contract_tests.cpp
)
target_include_directories(kernel_sdk_contract_tests PRIVATE tests/support include)
target_link_libraries(kernel_sdk_contract_tests PRIVATE bruce_lhme_core)
add_test(NAME kernel_sdk_contract_tests COMMAND kernel_sdk_contract_tests)

add_executable(kevin_grid_tests
  tests/support/test_main.cpp
  tests/kevin/kevin_grid_tests.cpp
)
target_include_directories(kevin_grid_tests PRIVATE tests/support include)
target_link_libraries(kevin_grid_tests PRIVATE bruce_lhme_core)
add_test(NAME kevin_grid_tests COMMAND kevin_grid_tests)

add_executable(kevin_domain_tests
  tests/support/test_main.cpp
  tests/kevin/kevin_domain_tests.cpp
)
target_include_directories(kevin_domain_tests PRIVATE tests/support include)
target_link_libraries(kevin_domain_tests PRIVATE bruce_lhme_core)
add_test(NAME kevin_domain_tests COMMAND kevin_domain_tests)

add_executable(kevin_memory_store_tests
  tests/support/test_main.cpp
  tests/kevin/kevin_memory_store_tests.cpp
)
target_include_directories(kevin_memory_store_tests PRIVATE tests/support include)
target_link_libraries(kevin_memory_store_tests PRIVATE bruce_lhme_core)
add_test(NAME kevin_memory_store_tests COMMAND kevin_memory_store_tests)

add_executable(kevin_snapshot_tests tests/support/test_main.cpp tests/kevin/kevin_snapshot_tests.cpp)
target_include_directories(kevin_snapshot_tests PRIVATE tests/support include)
target_link_libraries(kevin_snapshot_tests PRIVATE bruce_lhme_core)
add_test(NAME kevin_snapshot_tests COMMAND kevin_snapshot_tests)
add_executable(kevin_filter_tests tests/support/test_main.cpp tests/kevin/kevin_filter_tests.cpp)
target_include_directories(kevin_filter_tests PRIVATE tests/support include)
target_link_libraries(kevin_filter_tests PRIVATE bruce_lhme_core)
add_test(NAME kevin_filter_tests COMMAND kevin_filter_tests)

add_executable(lhme_kevin_memory_store_demo demos/lhme/kevin_memory_store_demo.cpp)
target_include_directories(lhme_kevin_memory_store_demo PRIVATE include demos/lhme/support)
target_link_libraries(lhme_kevin_memory_store_demo PRIVATE bruce_lhme_core)
add_test(NAME lhme_kevin_memory_store_demo COMMAND lhme_kevin_memory_store_demo)

add_executable(lhme_kevin_grid_filter_demo demos/lhme/kevin_grid_filter_demo.cpp)
target_include_directories(lhme_kevin_grid_filter_demo PRIVATE include demos/lhme/support)
target_link_libraries(lhme_kevin_grid_filter_demo PRIVATE bruce_lhme_core)
add_test(NAME lhme_kevin_grid_filter_demo COMMAND lhme_kevin_grid_filter_demo)

add_executable(lhme_technique_demo demos/lhme/technique_demo.cpp)
target_include_directories(lhme_technique_demo PRIVATE include demos/lhme/support)
target_link_libraries(lhme_technique_demo PRIVATE bruce_lhme_core)
add_test(NAME lhme_technique_demo COMMAND lhme_technique_demo)

add_executable(lhme_table_memory_benchmark
  demos/lhme/benchmarks/table_memory/table_memory_benchmark.cpp
)
target_include_directories(lhme_table_memory_benchmark PRIVATE
  include
  demos/lhme/support
  demos/lhme/benchmarks/table_memory
  demos/lhme/benchmarks/stress
)
target_link_libraries(lhme_table_memory_benchmark PRIVATE bruce_lhme_core)
add_test(NAME lhme_table_memory_benchmark COMMAND lhme_table_memory_benchmark)

add_executable(lhme_kevin_cpu_benchmark
  demos/lhme/benchmarks/kevin_cpu/kevin_cpu_benchmark.cpp
)
target_include_directories(lhme_kevin_cpu_benchmark PRIVATE
  include
  demos/lhme/support
  demos/lhme/benchmarks/table_memory
)
target_link_libraries(lhme_kevin_cpu_benchmark PRIVATE bruce_lhme_core)
add_test(NAME lhme_kevin_cpu_benchmark COMMAND lhme_kevin_cpu_benchmark)

add_executable(lhme_hybrid_storage_benchmark
  demos/lhme/benchmarks/hybrid_storage/hybrid_storage_benchmark.cpp
)
target_include_directories(lhme_hybrid_storage_benchmark PRIVATE
  include
  demos/lhme/support
  demos/lhme/benchmarks/table_memory
)
target_link_libraries(lhme_hybrid_storage_benchmark PRIVATE bruce_lhme_core)
add_test(NAME lhme_hybrid_storage_benchmark COMMAND lhme_hybrid_storage_benchmark)

add_executable(lhme_demo_report_tests demos/lhme/report_tests.cpp)
target_include_directories(lhme_demo_report_tests PRIVATE include demos/lhme/support)
target_link_libraries(lhme_demo_report_tests PRIVATE bruce_lhme_core)
add_test(NAME lhme_demo_report_tests COMMAND lhme_demo_report_tests)

include(CMakePackageConfigHelpers)

set(bruce_lhme_package_directory "${CMAKE_INSTALL_LIBDIR}/cmake/bruce_lhme")

install(TARGETS bruce_lhme_core
  EXPORT bruce_lhmeTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/bruce DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT bruce_lhmeTargets
  FILE bruce_lhmeTargets.cmake
  NAMESPACE bruce_lhme::
  DESTINATION ${bruce_lhme_package_directory}
)

configure_package_config_file(
  cmake/bruce_lhmeConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/bruce_lhmeConfig.cmake"
  INSTALL_DESTINATION ${bruce_lhme_package_directory}
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/bruce_lhmeConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/bruce_lhmeConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/bruce_lhmeConfigVersion.cmake"
  DESTINATION ${bruce_lhme_package_directory}
)

if(BUILD_TESTING)
  add_test(
    NAME lhme_package_layout
    COMMAND ${CMAKE_COMMAND}
      "-DBRUCE_BUILD_DIR=${CMAKE_BINARY_DIR}"
      "-DBRUCE_STAGE_DIR=${CMAKE_BINARY_DIR}/package_layout_stage"
      "-DBRUCE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/package_layout_test.cmake")

  add_test(
    NAME lhme_external_consumer
    COMMAND ${CMAKE_COMMAND}
      "-DBRUCE_BUILD_DIR=${CMAKE_BINARY_DIR}"
      "-DBRUCE_STAGE_DIR=${CMAKE_BINARY_DIR}/external_consumer_stage"
      "-DCONSUMER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/examples/external_consumer"
      "-DCONSUMER_BUILD_DIR=${CMAKE_BINARY_DIR}/external_consumer_build"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/external_consumer_test.cmake")

  add_test(
    NAME lhme_kernel_sdk_consumer
    COMMAND ${CMAKE_COMMAND}
      "-DBRUCE_BUILD_DIR=${CMAKE_BINARY_DIR}"
      "-DBRUCE_STAGE_DIR=${CMAKE_BINARY_DIR}/kernel_sdk_consumer_stage"
      "-DCONSUMER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/examples/kernel_sdk_consumer"
      "-DCONSUMER_BUILD_DIR=${CMAKE_BINARY_DIR}/kernel_sdk_consumer_build"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/kernel_sdk_consumer_test.cmake")

  add_custom_target(lhme_external_consumer
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "^lhme_external_consumer$"
    DEPENDS bruce_lhme_core
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endif()
