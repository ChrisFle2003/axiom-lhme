#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <vector>

#include "bruce/lhme/engine.hpp"
#include "bruce/lhme/kevin_grid.hpp"

namespace bruce::lhme {
enum class SnapshotStatus { Confirmed, IoError, InvalidSnapshot };
struct SnapshotOverrideRecord { GridIdentity identity; GridAddress address; BigInt value; };
struct KevinSnapshot { std::vector<GridSpec> grids; std::vector<SnapshotOverrideRecord> overrides; };
enum class KevinValueSource { Override, Computed };
struct KevinValue { EvaluationResult result; KevinValueSource source = KevinValueSource::Computed; };
struct KevinStoreStatistics {
  std::size_t registered_grid_count = 0;
  std::size_t override_count = 0;
  std::uint64_t computed_exact_evaluation_count = 0;
  CacheStatistics engine_cache;
};
// Legacy compatibility boundary. New code should keep GridDomainSpec and
// DomainProjection separate from FormulaOverrideStore. This type preserves the
// pre-separation GridSpec contract for existing callers and snapshots.
class KevinMemoryStore {
 public:
  explicit KevinMemoryStore(Engine& engine);
  bool register_grid(GridSpec spec);
  Status set_override(const GridIdentity&, const GridAddress&, BigInt);
  bool remove_override(const GridIdentity&, const GridAddress&);
  [[nodiscard]] KevinSnapshot snapshot() const;
  SnapshotStatus replace_snapshot(const KevinSnapshot&);
  [[nodiscard]] KevinValue get(const GridIdentity&, const GridAddress&, EvaluationMode,
                               Budget, std::optional<BigInt> modulus = std::nullopt);
  [[nodiscard]] KevinStoreStatistics inspect() const;
 private:
  Engine& engine_;
  std::map<std::string, GridSpec> grids_;
  std::map<std::string, SnapshotOverrideRecord> overrides_;
  std::atomic<std::uint64_t> computed_exact_evaluation_count_ = 0;
  std::shared_ptr<std::shared_mutex> mutex_;
};
}  // namespace bruce::lhme
