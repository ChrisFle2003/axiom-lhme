#include "axiom/lhme/reduction.hpp"

#include "axiom/lhme/trace.hpp"

#include <algorithm>
#include <future>
#include <limits>
#include <utility>

namespace axiom::lhme {
namespace {
struct TileResult { std::uint64_t id; RangeResult result; };

bool cancellation_requested(const EvaluationRequest& request) {
  return request.cancellation_requested ||
         (request.cancellation_signal != nullptr &&
          request.cancellation_signal->load());
}

TileResult evaluate_tile(std::uint64_t id, const EvaluationRequest& request,
                         AddressRange range) {
  TileResult tile{.id = id};
  for (std::int64_t index = range.start;; ++index) {
    if (cancellation_requested(request)) {
      tile.result.status = Status::Cancelled;
      tile.result.candidates.clear();
      return tile;
    }
    ++tile.result.summary.checked_count;
    auto address_request = request;
    address_request.address.index = index;
    auto value = evaluate(address_request);
    if (value.status == Status::Confirmed) {
      ++tile.result.summary.confirmed_count;
      tile.result.candidates.push_back(std::move(value));
    } else if (value.status == Status::InvalidArgument) {
      ++tile.result.summary.invalid_argument_count;
    }
    if (index == range.end) break;
  }
  return tile;
}
}  // namespace

RangeResult evaluate_range(const EvaluationRequest& request, AddressRange range,
                           CpuExecutionOptions options) {
  RangeResult output;
  if (cancellation_requested(request)) {
    output.status = Status::Cancelled;
    return output;
  }
  if (range.end < range.start || options.worker_count == 0 ||
      options.tile_size == 0 ||
      options.tile_size > static_cast<std::uint64_t>(
                              std::numeric_limits<std::int64_t>::max())) {
    output.status = Status::InvalidArgument;
    return output;
  }
  const auto range_count = static_cast<std::uint64_t>(range.end) -
                           static_cast<std::uint64_t>(range.start) + 1U;
  if (range_count == 0 || range_count > request.budget.max_candidate_count) {
    output.status = Status::BudgetExceeded;
    return output;
  }

  std::int64_t next_start = range.start;
  std::uint64_t next_id = 0;
  bool exhausted = false;
  while (!exhausted) {
    std::vector<std::future<TileResult>> batch;
    for (std::uint32_t slot = 0; slot < options.worker_count && !exhausted;
         ++slot) {
      const auto remaining = static_cast<std::uint64_t>(range.end) -
                             static_cast<std::uint64_t>(next_start) + 1U;
      const auto width = std::min(options.tile_size, remaining);
      const auto tile_end = next_start + static_cast<std::int64_t>(width - 1U);
      batch.push_back(std::async(std::launch::async, evaluate_tile, next_id++,
                                 std::cref(request),
                                 AddressRange{next_start, tile_end}));
      exhausted = tile_end == range.end;
      if (!exhausted) next_start = tile_end + 1;
    }
    for (auto& future : batch) {
      auto tile = future.get();
      if (tile.result.status == Status::Cancelled ||
          cancellation_requested(request)) {
        output.status = Status::Cancelled;
        output.candidates.clear();
        return output;
      }
      output.summary.checked_count += tile.result.summary.checked_count;
      output.summary.confirmed_count += tile.result.summary.confirmed_count;
      output.summary.invalid_argument_count += tile.result.summary.invalid_argument_count;
      for (auto& candidate : tile.result.candidates) {
        output.candidates.push_back(std::move(candidate));
      }
    }
  }
  return output;
}
}  // namespace axiom::lhme
