#pragma once

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <map>
#include <memory>
#include <shared_mutex>

#include "bruce/lhme/cache.hpp"
#include "bruce/lhme/reduction.hpp"
#include "bruce/lhme/technique.hpp"

namespace bruce::lhme {

struct EngineConfig {
  std::size_t cache_max_entries = 0;
  std::size_t cache_max_bytes = 0;
  std::uint32_t worker_count = 1;
  std::uint64_t tile_size = 1;
};

class Engine {
 public:
  [[nodiscard]] static Engine create(EngineConfig config);

  FormulaSpec register_formula(FormulaSpec spec);
  [[nodiscard]] bool register_technique(
      std::shared_ptr<const TechniqueKernel> kernel);
  [[nodiscard]] bool has_technique(const TechniqueIdentity& identity) const;
  [[nodiscard]] EvaluationResult evaluate(const EvaluationRequest& request);
  [[nodiscard]] EvaluationResult evaluate_technique(
      const TechniqueRequest& request);
  [[nodiscard]] RangeResult evaluate_range(const EvaluationRequest& request,
                                           AddressRange range);
  [[nodiscard]] CacheStatistics inspect() const;
  void cancel() noexcept;

 private:
  explicit Engine(EngineConfig config);
  [[nodiscard]] bool is_registered(const FormulaSpec& spec) const;

  EngineConfig config_;
  ResultCache cache_;
  std::map<std::string, FormulaSpec> formulas_;
  std::map<std::string, std::shared_ptr<const TechniqueKernel>> techniques_;
  std::shared_ptr<std::atomic_bool> cancellation_signal_;
  std::shared_ptr<std::shared_mutex> registry_mutex_;
};

}  // namespace bruce::lhme
