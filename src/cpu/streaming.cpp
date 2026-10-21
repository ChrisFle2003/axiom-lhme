#include "bruce/lhme/streaming.hpp"

#include "bruce/lhme/trace.hpp"

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <limits>
#include <mutex>
#include <thread>
#include <utility>

namespace bruce::lhme {
namespace {

struct Tile {
  std::uint64_t id = 0;
  AddressRange range;
};

struct TileResult {
  std::uint64_t id = 0;
  Status status = Status::Confirmed;
  GlobalSummary summary;
  std::uint64_t filtered_out_count = 0;
  std::uint64_t exact_evaluation_count = 0;
  std::vector<EvaluationResult> candidates;
};

struct TileCandidateLimits {
  bool retain_candidates = false;
  std::uint64_t max_count = 0;
  std::uint64_t max_bytes = 0;
};

bool cancellation_requested(const EvaluationRequest& request) {
  return request.cancellation_requested ||
         (request.cancellation_signal != nullptr &&
          request.cancellation_signal->load());
}

bool valid_filter(const CandidateFilter& filter) {
  switch (filter.mode) {
    case FilterMode::AcceptAll:
      return true;
    case FilterMode::ModularEquals:
      return filter.modulus > 0 && filter.expected_residue >= 0 &&
             filter.expected_residue < filter.modulus;
  }
  return false;
}

bool valid_retention_mode(RetentionMode mode) {
  switch (mode) {
    case RetentionMode::SummaryOnly:
    case RetentionMode::FilteredCandidates:
    case RetentionMode::TopK:
    case RetentionMode::Adaptive:
      return true;
  }
  return false;
}

std::uint64_t range_size(AddressRange range) {
  return static_cast<std::uint64_t>(range.end) -
             static_cast<std::uint64_t>(range.start) +
         1U;
}

std::uint64_t retained_size(const EvaluationResult& candidate) {
  std::uint64_t bytes = sizeof(EvaluationResult);
  if (candidate.integer.has_value()) {
    bytes += candidate.integer->convert_to<std::string>().size();
  }
  if (candidate.trace != nullptr) {
    for (const auto& event : candidate.trace->events) bytes += event.reason.size();
  }
  return bytes;
}

TileResult evaluate_tile(Tile tile, const EvaluationRequest& exact_request,
                         const CandidateFilter& filter,
                         TileCandidateLimits candidate_limits) {
  TileResult result{.id = tile.id};
  std::uint64_t buffered_bytes = 0;
  for (std::int64_t index = tile.range.start;; ++index) {
    if (cancellation_requested(exact_request)) {
      result.status = Status::Cancelled;
      result.candidates.clear();
      return result;
    }
    ++result.summary.checked_count;
    auto request = exact_request;
    request.address.index = index;

    if (filter.mode == FilterMode::ModularEquals) {
      request.mode = EvaluationMode::Modular;
      request.modulus = filter.modulus;
      const auto modular = evaluate(request);
      if (modular.status == Status::Cancelled) {
        result.status = Status::Cancelled;
        result.candidates.clear();
        return result;
      }
      if (modular.status == Status::InvalidArgument) {
        ++result.summary.invalid_argument_count;
        if (index == tile.range.end) break;
        continue;
      }
      if (modular.status != Status::Confirmed || !modular.integer.has_value()) {
        result.status = modular.status;
        result.candidates.clear();
        return result;
      }
      if (*modular.integer != filter.expected_residue) {
        ++result.filtered_out_count;
        if (index == tile.range.end) break;
        continue;
      }
    }

    request.mode = EvaluationMode::Exact;
    request.modulus.reset();
    ++result.exact_evaluation_count;
    auto exact = evaluate(request);
    if (exact.status == Status::Confirmed) {
      ++result.summary.confirmed_count;
      const auto candidate_bytes = retained_size(exact);
      if (candidate_limits.retain_candidates &&
          result.candidates.size() < candidate_limits.max_count &&
          candidate_bytes <= candidate_limits.max_bytes - buffered_bytes) {
        buffered_bytes += candidate_bytes;
        result.candidates.push_back(std::move(exact));
      }
    } else if (exact.status == Status::InvalidArgument) {
      ++result.summary.invalid_argument_count;
    } else {
      result.status = exact.status;
      result.candidates.clear();
      return result;
    }
    if (index == tile.range.end) break;
  }
  return result;
}

class CpuWorkerPool {
 public:
  explicit CpuWorkerPool(std::uint32_t worker_count) {
    workers_.reserve(worker_count);
    for (std::uint32_t index = 0; index < worker_count; ++index) {
      workers_.emplace_back([this] { run(); });
    }
  }

  CpuWorkerPool(const CpuWorkerPool&) = delete;
  auto operator=(const CpuWorkerPool&) -> CpuWorkerPool& = delete;

  ~CpuWorkerPool() {
    {
      const std::lock_guard lock(mutex_);
      stopping_ = true;
    }
    work_ready_.notify_all();
    for (auto& worker : workers_) worker.join();
  }

  void submit(std::vector<Tile> batch, const EvaluationRequest& request,
              const CandidateFilter& filter, TileCandidateLimits candidate_limits) {
    {
      const std::lock_guard lock(mutex_);
      request_ = &request;
      filter_ = &filter;
      candidate_limits_ = candidate_limits;
      pending_ = batch.size();
      for (auto& tile : batch) work_.push_back(std::move(tile));
    }
    work_ready_.notify_all();
  }

  [[nodiscard]] std::vector<TileResult> collect() {
    std::unique_lock lock(mutex_);
    completed_.wait(lock, [this] { return pending_ == 0; });
    auto completed = std::move(completed_results_);
    completed_results_.clear();
    return completed;
  }

 private:
  void run() {
    for (;;) {
      Tile tile;
      const EvaluationRequest* request = nullptr;
      const CandidateFilter* filter = nullptr;
      TileCandidateLimits candidate_limits;
      {
        std::unique_lock lock(mutex_);
        work_ready_.wait(lock, [this] { return stopping_ || !work_.empty(); });
        if (stopping_ && work_.empty()) return;
        tile = std::move(work_.front());
        work_.pop_front();
        request = request_;
        filter = filter_;
        candidate_limits = candidate_limits_;
      }

      TileResult completed;
      try {
        completed = evaluate_tile(std::move(tile), *request, *filter,
                                  candidate_limits);
      } catch (...) {
        completed.id = tile.id;
        completed.status = Status::InternalError;
        completed.candidates.clear();
      }

      {
        const std::lock_guard lock(mutex_);
        completed_results_.push_back(std::move(completed));
        --pending_;
      }
      completed_.notify_one();
    }
  }

  std::mutex mutex_;
  std::condition_variable work_ready_;
  std::condition_variable completed_;
  bool stopping_ = false;
  std::size_t pending_ = 0;
  const EvaluationRequest* request_ = nullptr;
  const CandidateFilter* filter_ = nullptr;
  TileCandidateLimits candidate_limits_;
  std::deque<Tile> work_;
  std::vector<TileResult> completed_results_;
  std::vector<std::thread> workers_;
};

bool can_retain(const StreamingRangeResult& output,
                const StreamingOptions& options, std::uint64_t used_bytes,
                const EvaluationResult& candidate) {
  if (output.retained_candidates.size() >= options.max_retained_candidates) {
    return false;
  }
  const auto candidate_bytes = retained_size(candidate);
  return candidate_bytes <= options.retention_memory_bytes - used_bytes;
}

bool merge_tile(StreamingRangeResult& output, TileResult tile,
                const StreamingOptions& options, std::uint64_t& used_bytes,
                bool& adaptive_switched, const EvaluationRequest& request) {
  output.summary.checked_count += tile.summary.checked_count;
  output.summary.confirmed_count += tile.summary.confirmed_count;
  output.summary.invalid_argument_count += tile.summary.invalid_argument_count;
  output.filtered_out_count += tile.filtered_out_count;
  output.exact_evaluation_count += tile.exact_evaluation_count;

  for (auto& candidate : tile.candidates) {
    if (cancellation_requested(request)) return false;
    if (output.effective_retention_mode == RetentionMode::SummaryOnly) continue;
    const bool fits = can_retain(output, options, used_bytes, candidate);
    if (!fits) {
      if (options.retention_mode == RetentionMode::Adaptive) {
        adaptive_switched = true;
        output.effective_retention_mode = RetentionMode::TopK;
      }
      continue;
    }
    used_bytes += retained_size(candidate);
    output.retained_candidates.push_back(std::move(candidate));
  }
  return !cancellation_requested(request);
}

}  // namespace

StreamingRangeResult evaluate_streaming_range(const EvaluationRequest& exact_request,
                                              AddressRange range,
                                              StreamingOptions options) {
  StreamingRangeResult output;
  if (exact_request.mode != EvaluationMode::Exact || range.end < range.start ||
      options.worker_count == 0 || options.tile_size == 0 ||
      options.tile_size > static_cast<std::uint64_t>(
                              std::numeric_limits<std::int64_t>::max()) ||
      !valid_retention_mode(options.retention_mode) || !valid_filter(options.filter)) {
    output.status = Status::InvalidArgument;
    return output;
  }
  if (cancellation_requested(exact_request)) {
    output.status = Status::Cancelled;
    return output;
  }
  const auto count = range_size(range);
  if (count == 0 || count > exact_request.budget.max_candidate_count) {
    output.status = Status::BudgetExceeded;
    return output;
  }

  if (options.retention_mode == RetentionMode::SummaryOnly ||
      options.max_retained_candidates == 0 || options.retention_memory_bytes == 0) {
    output.effective_retention_mode = RetentionMode::SummaryOnly;
  } else if (options.retention_mode == RetentionMode::Adaptive) {
    output.effective_retention_mode = RetentionMode::FilteredCandidates;
  } else {
    output.effective_retention_mode = options.retention_mode;
  }

  CpuWorkerPool pool(options.worker_count);
  std::int64_t next_start = range.start;
  std::uint64_t next_tile_id = 0;
  bool exhausted = false;
  bool adaptive_switched = false;
  std::uint64_t used_bytes = 0;
  const TileCandidateLimits candidate_limits{
      .retain_candidates = output.effective_retention_mode != RetentionMode::SummaryOnly,
      .max_count = options.max_retained_candidates,
      .max_bytes = options.retention_memory_bytes};
  const std::uint32_t batch_slots = candidate_limits.retain_candidates
                                        ? 1U
                                        : options.worker_count;
  while (!exhausted) {
    if (cancellation_requested(exact_request)) {
      output.status = Status::Cancelled;
      output.retained_candidates.clear();
      return output;
    }
    std::vector<Tile> batch;
    batch.reserve(batch_slots);
    for (std::uint32_t slot = 0; slot < batch_slots && !exhausted;
         ++slot) {
      const auto remaining = static_cast<std::uint64_t>(range.end) -
                             static_cast<std::uint64_t>(next_start) + 1U;
      const auto width = std::min(options.tile_size, remaining);
      const auto end = next_start + static_cast<std::int64_t>(width - 1U);
      batch.push_back({.id = next_tile_id++, .range = {next_start, end}});
      exhausted = end == range.end;
      if (!exhausted) next_start = end + 1;
    }
    pool.submit(std::move(batch), exact_request, options.filter, candidate_limits);
    auto completed = pool.collect();
    std::sort(completed.begin(), completed.end(),
              [](const TileResult& left, const TileResult& right) {
                return left.id < right.id;
              });
    for (auto& tile : completed) {
      if (tile.status != Status::Confirmed || cancellation_requested(exact_request)) {
        output.status = tile.status == Status::Confirmed ? Status::Cancelled : tile.status;
        output.retained_candidates.clear();
        return output;
      }
      if (!merge_tile(output, std::move(tile), options, used_bytes,
                      adaptive_switched, exact_request)) {
        output.status = Status::Cancelled;
        output.retained_candidates.clear();
        return output;
      }
    }
  }
  if (cancellation_requested(exact_request)) {
    output.status = Status::Cancelled;
    output.retained_candidates.clear();
    return output;
  }
  if (adaptive_switched) output.effective_retention_mode = RetentionMode::TopK;
  output.status = Status::Confirmed;
  return output;
}

}  // namespace bruce::lhme
