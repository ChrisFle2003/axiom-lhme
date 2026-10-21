#pragma once

#include "fair_storage.hpp"
#include "process_memory.hpp"

#if defined(__linux__)
#include <cerrno>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace bruce::lhme::benchmark::fair {

struct IsolatedPeakMemory {
  bool available = false;
  bool success = false;
  std::uint64_t bytes = 0;
};

[[nodiscard]] inline IsolatedPeakMemory measure_storage_peak(
    const FairBenchmarkConfig& config, StorageStrategyKind strategy,
    const Workload& workload) {
#if defined(__linux__)
  struct Wire { std::uint64_t bytes; std::uint32_t success; };
  int descriptors[2]{};
  if (pipe(descriptors) != 0) return {.available = true};
  const auto child = fork();
  if (child == 0) {
    close(descriptors[0]);
    const auto storage = build_storage_case(config, strategy);
    const auto peak = storage.has_value() && verify_workload(*storage, workload)
        ? demo::stress::process_peak_memory() : demo::stress::ProcessPeakMemory{};
    const Wire wire{.bytes = peak.bytes, .success = peak.available ? 1U : 0U};
    const auto written = write(descriptors[1], &wire, sizeof(wire));
    close(descriptors[1]);
    _exit(written == static_cast<ssize_t>(sizeof(wire)) ? 0 : 1);
  }
  close(descriptors[1]);
  Wire wire{};
  const auto received = child < 0 ? -1 : read(descriptors[0], &wire, sizeof(wire));
  close(descriptors[0]);
  int status = 0;
  if (child < 0 || waitpid(child, &status, 0) < 0) return {.available = true};
  return {.available = true,
          .success = received == static_cast<ssize_t>(sizeof(wire)) &&
                     WIFEXITED(status) && WEXITSTATUS(status) == 0 && wire.success != 0,
          .bytes = wire.bytes};
#else
  static_cast<void>(config);
  static_cast<void>(strategy);
  static_cast<void>(workload);
  return {};
#endif
}

}  // namespace bruce::lhme::benchmark::fair
