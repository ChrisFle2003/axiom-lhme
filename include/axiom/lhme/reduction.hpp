#pragma once

#include <cstdint>
#include <vector>

#include "axiom/lhme/formula.hpp"

namespace axiom::lhme {

struct AddressRange { std::int64_t start = 0; std::int64_t end = -1; };
struct CpuExecutionOptions { std::uint32_t worker_count = 1; std::uint64_t tile_size = 1; };
struct GlobalSummary {
  std::uint64_t checked_count = 0;
  std::uint64_t confirmed_count = 0;
  std::uint64_t invalid_argument_count = 0;
  auto operator==(const GlobalSummary&) const -> bool = default;
};
struct RangeResult {
  Status status = Status::Confirmed;
  GlobalSummary summary;
  std::vector<EvaluationResult> candidates;
};

[[nodiscard]] RangeResult evaluate_range(const EvaluationRequest& request,
                                         AddressRange range,
                                         CpuExecutionOptions options);
}  // namespace axiom::lhme
