#pragma once

#include <cstdint>

#if defined(__linux__)
#include <sys/resource.h>
#endif

namespace bruce::lhme::demo::stress {

struct ProcessPeakMemory {
  bool available = false;
  std::uint64_t bytes = 0;
};

[[nodiscard]] inline ProcessPeakMemory process_peak_memory() noexcept {
#if defined(__linux__)
  rusage usage{};
  if (getrusage(RUSAGE_SELF, &usage) != 0 || usage.ru_maxrss < 0) return {};
  return {.available = true,
          .bytes = static_cast<std::uint64_t>(usage.ru_maxrss) * 1024U};
#else
  return {};
#endif
}

}  // namespace bruce::lhme::demo::stress
