#include "bruce/lhme/kevin_filter.hpp"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>

namespace bruce::lhme {

class KevinTileCpuExecutor::Impl {
 public:
  explicit Impl(std::uint32_t worker_count) : worker_count_(worker_count) {
    workers_.reserve(worker_count_);
    for (std::uint32_t worker = 0; worker < worker_count_; ++worker) {
      workers_.emplace_back([this] { run(); });
    }
  }

  ~Impl() {
    {
      const std::lock_guard lock(mutex_);
      stopping_ = true;
    }
    work_ready_.notify_all();
    for (auto& worker : workers_) worker.join();
  }

  [[nodiscard]] TileReductionSummary filter(
      KevinMemoryStore& store, const GridIdentity& grid,
      std::span<const GridAddress> parents, const BigInt& modulus,
      const BigInt& residue, std::uint64_t minimum, std::uint64_t maximum) {
    const std::unique_lock call_lock(call_mutex_);
    TileReductionSummary output{.tile_count = parents.size()};
    if (worker_count_ == 0) {
      output.status = Status::InvalidArgument;
      return output;
    }
    if (parents.empty()) return output;

    std::vector<TileSummary> results(parents.size());
    {
      const std::lock_guard lock(mutex_);
      store_ = &store;
      grid_ = grid;
      parents_ = parents;
      modulus_ = modulus;
      residue_ = residue;
      minimum_ = minimum;
      maximum_ = maximum;
      results_ = &results;
      next_index_ = 0;
      remaining_ = parents.size();
      failure_ = Status::Confirmed;
      active_ = true;
    }
    work_ready_.notify_all();

    {
      std::unique_lock lock(mutex_);
      completed_.wait(lock, [this] { return !active_; });
      output.status = failure_;
    }
    if (output.status != Status::Confirmed) return output;

    for (const auto& tile : results) {
      output.checked += tile.checked;
      output.modular_survivors += tile.modular_survivors;
      output.metric_survivors += tile.metric_survivors;
      output.final_survivors += tile.final_survivors;
      output.candidates.insert(output.candidates.end(), tile.candidates.begin(),
                               tile.candidates.end());
    }
    return output;
  }

 private:
  void run() {
    for (;;) {
      std::size_t index = 0;
      KevinMemoryStore* store = nullptr;
      GridIdentity grid;
      BigInt modulus;
      BigInt residue;
      std::uint64_t minimum = 0;
      std::uint64_t maximum = 0;
      GridAddress parent = *GridAddress::from_steps({});
      {
        std::unique_lock lock(mutex_);
        work_ready_.wait(lock, [this] {
          return stopping_ || (active_ && next_index_ < parents_.size());
        });
        if (stopping_) return;
        index = next_index_++;
        store = store_;
        grid = grid_;
        modulus = modulus_;
        residue = residue_;
        minimum = minimum_;
        maximum = maximum_;
        parent = parents_[index];
      }

      TileSummary result;
      Status failure = Status::Confirmed;
      try {
        result = filter_tile(*store, grid, parent, modulus, residue, minimum,
                             maximum);
      } catch (...) {
        failure = Status::InternalError;
      }

      {
        const std::lock_guard lock(mutex_);
        if (failure_ == Status::Confirmed && failure != Status::Confirmed) {
          failure_ = failure;
        }
        (*results_)[index] = std::move(result);
        --remaining_;
        if (remaining_ == 0) {
          active_ = false;
          completed_.notify_one();
        }
      }
    }
  }

  std::uint32_t worker_count_ = 0;
  std::mutex call_mutex_;
  std::mutex mutex_;
  std::condition_variable work_ready_;
  std::condition_variable completed_;
  bool stopping_ = false;
  bool active_ = false;
  KevinMemoryStore* store_ = nullptr;
  GridIdentity grid_;
  std::span<const GridAddress> parents_;
  BigInt modulus_ = 0;
  BigInt residue_ = 0;
  std::uint64_t minimum_ = 0;
  std::uint64_t maximum_ = 0;
  std::vector<TileSummary>* results_ = nullptr;
  std::size_t next_index_ = 0;
  std::size_t remaining_ = 0;
  Status failure_ = Status::Confirmed;
  std::vector<std::thread> workers_;
};

KevinTileCpuExecutor::KevinTileCpuExecutor(std::uint32_t worker_count)
    : impl_(std::make_unique<Impl>(worker_count)) {}

KevinTileCpuExecutor::~KevinTileCpuExecutor() = default;

TileReductionSummary KevinTileCpuExecutor::filter(
    KevinMemoryStore& store, const GridIdentity& grid,
    std::span<const GridAddress> parents, const BigInt& modulus,
    const BigInt& residue, std::uint64_t minimum, std::uint64_t maximum) {
  return impl_->filter(store, grid, parents, modulus, residue, minimum, maximum);
}

}  // namespace bruce::lhme
