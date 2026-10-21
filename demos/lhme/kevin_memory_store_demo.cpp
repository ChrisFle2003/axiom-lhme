#include "demo_report.hpp"
#include "bruce/lhme/domain_projection.hpp"
#include "bruce/lhme/kevin_memory_store.hpp"
#include "bruce/lhme/power_offset_kernel.hpp"
#include <memory>
#include <vector>
int main() {
  using namespace bruce::lhme;
  demo::Report report; report.heading("KEVIN Memory Store");
  auto engine = Engine::create({.cache_max_entries=8, .cache_max_bytes=4096});
  report.expect_true("kernel registered",
                     engine.register_technique(std::make_shared<PowerOffsetKernel>()));
  KevinMemoryStore store{engine};
  GridSpec grid{{.id="bruce.demo.kevin",.version=1}, {.id="bruce.power_offset",.version=1}, encode_power_offset_parameters(2,100), 10};
  report.expect_true("grid registered", store.register_grid(grid));
  report.expect_equal("cache before read", std::size_t(0), store.inspect().engine_cache.resident_entries);
  std::vector<GridStep> steps(10, GridStep{.x=7,.y=7}); auto address=*GridAddress::from_steps(steps);
  DepthPathProjection projection;
  auto metric=store.get(grid.identity,address,EvaluationMode::Metric,Budget::unlimited());
  report.expect_equal("metric status", Status::Confirmed, metric.result.status);
  report.row("grid depth", std::to_string(address.depth()));
  report.row("legacy depth-path formula index", std::to_string(projection.project(address)->index));
  report.row("source", metric.source==KevinValueSource::Computed ? "computed" : "override");
  report.row("cache entries", std::to_string(store.inspect().engine_cache.resident_entries));
  return report.finish();
}
