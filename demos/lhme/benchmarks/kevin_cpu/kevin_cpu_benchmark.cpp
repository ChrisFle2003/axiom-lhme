#include "demo_report.hpp"

#include "axiom/lhme/domain_projection.hpp"
#include "axiom/lhme/kevin_filter.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <future>
#include <string>
#include <utility>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;
constexpr std::uint32_t kTileCount = 64;
constexpr std::array<std::uint32_t, 4> kWorkerCounts{1, 2, 4, 8};
constexpr std::uint32_t kRunsPerExecutor = 4;

struct FilterMeasurement {
  axiom::lhme::TileReductionSummary summary;
  std::int64_t elapsed_microseconds = 0;
};

class EngineFormulaEvaluator final : public axiom::lhme::FormulaEvaluator {
 public:
  EngineFormulaEvaluator(axiom::lhme::Engine& engine, axiom::lhme::FormulaSpec spec)
      : engine_(engine), spec_(std::move(spec)) {}

  axiom::lhme::EvaluationResult evaluate(
      axiom::lhme::FormulaAddress address, axiom::lhme::EvaluationMode mode,
      axiom::lhme::Budget budget,
      std::optional<axiom::lhme::BigInt> modulus) override {
    return engine_.evaluate({.spec = spec_, .address = address, .mode = mode,
                             .budget = budget, .modulus = std::move(modulus)});
  }

 private:
  axiom::lhme::Engine& engine_;
  axiom::lhme::FormulaSpec spec_;
};

void add_tile(axiom::lhme::TileReductionSummary& total,
              const axiom::lhme::TileSummary& tile) {
  if (tile.status != axiom::lhme::Status::Confirmed) total.status = tile.status;
  total.checked += tile.checked;
  total.modular_survivors += tile.modular_survivors;
  total.metric_survivors += tile.metric_survivors;
  total.final_survivors += tile.final_survivors;
  total.candidates.insert(total.candidates.end(), tile.candidates.begin(),
                          tile.candidates.end());
}

[[nodiscard]] std::vector<axiom::lhme::GridAddress> make_parent_tiles() {
  using namespace axiom::lhme;
  std::vector<GridAddress> parents;
  parents.reserve(kTileCount);
  for (std::uint8_t y = 0; y < 8; ++y) {
    for (std::uint8_t x = 0; x < 8; ++x) {
      const std::vector<GridStep> steps{{.x = x, .y = y}};
      parents.push_back(*GridAddress::from_steps(steps));
    }
  }
  return parents;
}

[[nodiscard]] FilterMeasurement measure_filter(std::uint32_t worker_count,
                                                const std::vector<axiom::lhme::GridAddress>& parents) {
  using namespace axiom::lhme;
  auto engine = Engine::create({});
  const auto formula = engine.register_formula(make_power_offset_spec(2, 100, 1));
  EngineFormulaEvaluator evaluator{engine, formula};
  const GridDomainSpec domain{{.id = "axiom.benchmark.cpu", .version = 1}, 2};
  DepthPathProjection projection;
  const DomainEvaluationContext context{.domain = domain, .projection = &projection,
                                        .evaluator = evaluator};
  TileReductionSummary summary;
  const auto started = Clock::now();
  for (std::uint32_t run = 0; run < kRunsPerExecutor; ++run) {
    summary = {.tile_count = parents.size()};
    for (std::size_t next = 0; next < parents.size();) {
      std::vector<std::future<TileSummary>> batch;
      const auto batch_size = std::min<std::size_t>(worker_count, parents.size() - next);
      batch.reserve(batch_size);
      for (std::size_t slot = 0; slot < batch_size; ++slot) {
        const auto parent = parents[next++];
        batch.push_back(std::async(std::launch::async, [&context, parent] {
          return filter_tile(context, parent, /*modulus=*/17, /*residue=*/11,
                             /*minimum bits=*/7, /*maximum bits=*/5'000);
        }));
      }
      for (auto& future : batch) add_tile(summary, future.get());
    }
    if (summary.status != Status::Confirmed) break;
  }
  const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
      Clock::now() - started);
  return {.summary = std::move(summary),
          .elapsed_microseconds = elapsed.count() /
                                  static_cast<std::int64_t>(kRunsPerExecutor)};
}

}  // namespace

int main() {
  using namespace axiom::lhme;

  demo::Report report;
  report.heading("KEVIN Domain Projection and Filter Overhead");
  report.row("comparison rule",
             "KEVIN supplies only 8x8 domain coordinates and explicit projection/filter work; FormulaRecipe owns values");
  report.row("timing rule", "average of four reductions with bounded asynchronous CPU batches");

  const auto parents = make_parent_tiles();
  std::optional<FilterMeasurement> reference;
  for (const auto worker_count : kWorkerCounts) {
    const auto measurement = measure_filter(worker_count, parents);
    const auto label = "KEVIN " + std::to_string(worker_count) + " worker";
    report.row(label + " checked virtual cells",
               std::to_string(measurement.summary.checked));
    report.row(label + " survivors", std::to_string(measurement.summary.final_survivors));
    report.row(label + " exact evaluations", std::to_string(measurement.summary.exact_evaluations));
    report.row(label + " elapsed microseconds", std::to_string(measurement.elapsed_microseconds));
    report.expect_equal(label + " status", Status::Confirmed, measurement.summary.status);
    report.expect_equal(label + " tile count", std::uint64_t(kTileCount),
                        measurement.summary.tile_count);
    report.expect_equal(label + " checked cells", std::uint64_t(kTileCount) * 64U,
                        measurement.summary.checked);
    report.expect_equal(label + " filter makes no exact work", std::uint64_t(0),
                        measurement.summary.exact_evaluations);
    if (!reference.has_value()) {
      reference = measurement;
    } else {
      report.expect_equal(label + " deterministic survivors",
                          reference->summary.final_survivors,
                          measurement.summary.final_survivors);
      report.expect_true(label + " deterministic addresses",
                         reference->summary.candidates == measurement.summary.candidates);
    }
  }

  return report.finish();
}
