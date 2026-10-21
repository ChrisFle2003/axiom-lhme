#pragma once

#include "isolated_protocol.hpp"

#include <string>
#include <string_view>

#if defined(__linux__)
#include <cerrno>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace bruce::lhme::demo::stress {

struct IsolatedRunResult {
  bool available = false;
  bool success = false;
  ChildReport report{};
};

#if defined(__linux__)
namespace detail {

[[nodiscard]] inline bool read_report(int file_descriptor, ChildReport& report) {
  auto* output = reinterpret_cast<unsigned char*>(&report);
  std::size_t remaining = sizeof(report);
  while (remaining > 0) {
    const auto read_count = read(file_descriptor, output, remaining);
    if (read_count == 0) return false;
    if (read_count < 0) {
      if (errno == EINTR) continue;
      return false;
    }
    output += read_count;
    remaining -= static_cast<std::size_t>(read_count);
  }
  return true;
}

[[nodiscard]] inline bool stream_has_no_extra_bytes(int file_descriptor) {
  unsigned char byte = 0;
  while (true) {
    const auto read_count = read(file_descriptor, &byte, sizeof(byte));
    if (read_count == 0) return true;
    if (read_count < 0 && errno == EINTR) continue;
    return false;
  }
}

[[nodiscard]] inline bool exited_successfully(pid_t child) {
  int status = 0;
  while (waitpid(child, &status, 0) < 0) {
    if (errno != EINTR) return false;
  }
  return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

}  // namespace detail
#endif

[[nodiscard]] inline IsolatedRunResult run_isolated(std::string_view executable,
                                                     Scenario scenario) {
#if defined(__linux__)
  IsolatedRunResult output{.available = true};
  if (executable.empty()) return output;

  int pipe_descriptors[2]{};
  if (pipe(pipe_descriptors) != 0) return output;

  const auto child = fork();
  if (child < 0) {
    close(pipe_descriptors[0]);
    close(pipe_descriptors[1]);
    return output;
  }

  if (child == 0) {
    close(pipe_descriptors[0]);
    if (dup2(pipe_descriptors[1], STDOUT_FILENO) < 0) _exit(126);
    close(pipe_descriptors[1]);

    const std::string executable_path{executable};
    const auto name = scenario_name(scenario);
    char* const arguments[] = {const_cast<char*>(executable_path.c_str()),
                               const_cast<char*>("--isolated-child"),
                               const_cast<char*>(name.data()), nullptr};
    execv(executable_path.c_str(), arguments);
    _exit(127);
  }

  close(pipe_descriptors[1]);
  const bool received_report = detail::read_report(pipe_descriptors[0], output.report);
  const bool no_extra_bytes = received_report &&
                              detail::stream_has_no_extra_bytes(pipe_descriptors[0]);
  close(pipe_descriptors[0]);
  const bool child_exited = detail::exited_successfully(child);

  output.success = received_report && no_extra_bytes && child_exited &&
                   valid_child_report(output.report) &&
                   output.report.scenario == static_cast<std::uint32_t>(scenario) &&
                   output.report.status == static_cast<std::uint32_t>(Status::Confirmed);
  return output;
#else
  static_cast<void>(executable);
  static_cast<void>(scenario);
  return {};
#endif
}

}  // namespace bruce::lhme::demo::stress
