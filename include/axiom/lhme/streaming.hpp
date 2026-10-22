#pragma once

#include <cstdint>
#include <vector>

#include "axiom/lhme/reduction.hpp"

namespace axiom::lhme {

enum class RetentionMode {
  SummaryOnly,
  FilteredCandidates,
  TopK,
  Adaptive,
};

enum class FilterMode {
  AcceptAll,
  ModularEquals,
};

struct CandidateFilter {
  FilterMode mode = FilterMode::AcceptAll;
  BigInt modulus = 1;
  BigInt expected_residue = 0;
};

struct StreamingOptions {
  std::uint32_t worker_count = 1;
  std::uint64_t tile_size = 1;
  RetentionMode retention_mode = RetentionMode::Adaptive;
  std::uint64_t max_retained_candidates = 0;
  std::uint64_t retention_memory_bytes = 0;
  CandidateFilter filter;
};

struct StreamingRangeResult {
  Status status = Status::InternalError;
  GlobalSummary summary;
  std::vector<EvaluationResult> retained_candidates;
  RetentionMode effective_retention_mode = RetentionMode::SummaryOnly;
  std::uint64_t filtered_out_count = 0;
  std::uint64_t exact_evaluation_count = 0;
};

[[nodiscard]] StreamingRangeResult evaluate_streaming_range(
    const EvaluationRequest& exact_request, AddressRange range,
    StreamingOptions options);

}  // namespace axiom::lhme
