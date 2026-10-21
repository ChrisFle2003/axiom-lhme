#include "test.hpp"
#include "bruce/lhme/domain_projection.hpp"
#include "bruce/lhme/formula_evaluator.hpp"
#include "bruce/lhme/kevin_domain.hpp"
#include "bruce/lhme/kevin_filter.hpp"
#include "bruce/lhme/power_offset_kernel.hpp"
#include "bruce/lhme/trace.hpp"
#include <memory>
#include <vector>

namespace {

using namespace bruce::lhme;

class RecordingEvaluator final : public FormulaEvaluator {
 public:
  std::vector<std::int64_t> addresses;

  EvaluationResult evaluate(FormulaAddress address, EvaluationMode mode,
                            Budget, std::optional<BigInt> modulus) override {
    addresses.push_back(address.index);
    EvaluationResult result;
    result.status = Status::Confirmed;
    result.mode = mode;
    if (mode == EvaluationMode::Modular) result.integer = address.index % *modulus;
    if (mode == EvaluationMode::Metric) result.metric_bit_length = 8;
    if (mode == EvaluationMode::Exact) result.integer = address.index;
    return result;
  }
};

class OffsetProjection final : public DomainProjection {
 public:
  explicit OffsetProjection(std::int64_t offset) : offset_(offset) {}
  ProjectionIdentity identity() const override {
    return {.id = "bruce.test.offset", .version = 1};
  }
  std::optional<FormulaAddress> project(const GridAddress&) const override {
    return FormulaAddress{.index = offset_};
  }

 private:
  std::int64_t offset_;
};

GridDomainSpec domain() {
  return {{.id = "bruce.filter.domain", .version = 1}, 2};
}

}  // namespace

TEST(domain_filter_rejects_a_missing_projection) {
  RecordingEvaluator evaluator;
  const auto spec = domain();
  const DomainEvaluationContext context{.domain = spec, .projection = nullptr,
                                        .evaluator = evaluator};
  const auto root = *GridAddress::from_steps({});

  const auto result = filter_tile(context, root, 17, 11, 7, 17);

  CHECK(result.status == Status::InvalidArgument);
  CHECK(result.candidates.empty());
  CHECK(evaluator.addresses.empty());
}

TEST(domain_filter_uses_the_registered_projection_for_metric_and_modular_queries) {
  RecordingEvaluator evaluator;
  const auto spec = domain();
  DepthPathProjection projection;
  const DomainEvaluationContext context{.domain = spec, .projection = &projection,
                                        .evaluator = evaluator};
  const auto root = *GridAddress::from_steps({});

  const auto result = filter_tile(context, root, 17, 1, 7, 17);

  CHECK(result.status == Status::Confirmed);
  CHECK(result.checked == 64);
  CHECK(!evaluator.addresses.empty());
  CHECK(evaluator.addresses.front() == 1);
}

TEST(distinct_projections_map_one_grid_coordinate_to_distinct_formula_addresses) {
  RecordingEvaluator evaluator;
  const auto spec = domain();
  OffsetProjection first{17};
  OffsetProjection second{29};
  const DomainEvaluationContext first_context{.domain = spec, .projection = &first,
                                              .evaluator = evaluator};
  const DomainEvaluationContext second_context{.domain = spec, .projection = &second,
                                               .evaluator = evaluator};
  const std::vector<GridStep> steps{{.x = 3, .y = 5}};
  const auto address = *GridAddress::from_steps(steps);

  const auto first_result = evaluate_domain_address(
      first_context, address, EvaluationMode::Exact, Budget::unlimited());
  const auto second_result = evaluate_domain_address(
      second_context, address, EvaluationMode::Exact, Budget::unlimited());

  CHECK(first_result.status == Status::Confirmed);
  CHECK(second_result.status == Status::Confirmed);
  const std::vector<std::int64_t> expected{17, 29};
  CHECK(evaluator.addresses == expected);
}

TEST(domain_evaluation_trace_records_domain_projection_and_formula_address) {
  RecordingEvaluator evaluator;
  const auto spec = domain();
  DepthPathProjection projection;
  const DomainEvaluationContext context{.domain = spec, .projection = &projection,
                                        .evaluator = evaluator};
  const std::vector<GridStep> steps{{.x = 3, .y = 5}};
  const auto address = *GridAddress::from_steps(steps);

  const auto result = evaluate_domain_address(
      context, address, EvaluationMode::Exact, Budget::unlimited());

  CHECK(result.status == Status::Confirmed);
  CHECK(result.trace != nullptr);
  CHECK(result.trace->contains(GateScope::Projection));
  CHECK(result.trace->events.back().reason ==
        "domain=bruce.filter.domain@1;projection=bruce.depth-path@1;formula=44");
}

TEST(domain_evaluation_trace_explains_a_missing_projection) {
  RecordingEvaluator evaluator;
  const auto spec = domain();
  const DomainEvaluationContext context{.domain = spec, .projection = nullptr,
                                        .evaluator = evaluator};
  const auto address = *GridAddress::from_steps({});

  const auto result = evaluate_domain_address(
      context, address, EvaluationMode::Metric, Budget::unlimited());

  CHECK(result.status == Status::InvalidArgument);
  CHECK(result.trace != nullptr);
  CHECK(result.trace->contains(GateScope::Branch));
  CHECK(result.trace->events.back().reason ==
        "domain=bruce.filter.domain@1;projection=missing");
}

TEST(kevin_filter_maps_8x8_bits_stably){ using namespace bruce::lhme; CHECK(grid_tile_bit(0,0)==0); CHECK(grid_tile_bit(7,7)==63); CHECK((CandidateMask64::all()&CandidateMask64{1}).popcount()==1); CHECK((CandidateMask64{1}|CandidateMask64{2}).bits()==3); CHECK((CandidateMask64{3}^CandidateMask64{1}).bits()==2); CHECK(grid_tile_row_mask(1)==0xff00ULL); }
TEST(kevin_filter_uses_modular_without_exact_materialization){ using namespace bruce::lhme; auto engine=Engine::create({}); CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>())); KevinMemoryStore store{engine}; GridSpec grid{{.id="bruce.filter",.version=1},{.id="bruce.power_offset",.version=1},encode_power_offset_parameters(2,100),1}; CHECK(store.register_grid(grid)); const auto parent=*GridAddress::from_steps({}); const auto mask=modular_equals_tile(store,grid.identity,parent,17,11); CHECK(mask.popcount()>0); CHECK(store.inspect().computed_exact_evaluation_count==0); }
TEST(kevin_filter_uses_metric_without_exact_materialization){ using namespace bruce::lhme; auto engine=Engine::create({}); CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>())); KevinMemoryStore store{engine}; GridSpec grid{{.id="bruce.metric",.version=1},{.id="bruce.power_offset",.version=1},encode_power_offset_parameters(2,100),1}; CHECK(store.register_grid(grid)); const auto parent=*GridAddress::from_steps({}); const auto mask=metric_bit_length_between_tile(store,grid.identity,parent,7,17); CHECK(mask.popcount()>0); CHECK(store.inspect().computed_exact_evaluation_count==0); }
TEST(kevin_filter_pipeline_returns_only_lazy_survivors){ using namespace bruce::lhme; auto engine=Engine::create({}); CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>())); KevinMemoryStore store{engine}; GridSpec grid{{.id="bruce.pipeline",.version=1},{.id="bruce.power_offset",.version=1},encode_power_offset_parameters(2,100),1}; CHECK(store.register_grid(grid)); const auto parent=*GridAddress::from_steps({}); const auto summary=filter_tile(store,grid.identity,parent,17,11,7,17); CHECK(summary.checked==64); CHECK(summary.final_survivors==summary.candidates.size()); CHECK(summary.exact_evaluations==0); CHECK(store.inspect().computed_exact_evaluation_count==0); }
TEST(kevin_filter_exact_escalation_respects_candidate_limit){ using namespace bruce::lhme; auto engine=Engine::create({}); CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>())); KevinMemoryStore store{engine}; GridSpec grid{{.id="bruce.exact",.version=1},{.id="bruce.power_offset",.version=1},encode_power_offset_parameters(2,100),1}; CHECK(store.register_grid(grid)); const auto parent=*GridAddress::from_steps({}); const auto summary=filter_tile(store,grid.identity,parent,17,11,7,17); const auto none=evaluate_survivors_exact(store,grid.identity,summary,0,Budget::unlimited()); const auto one=evaluate_survivors_exact(store,grid.identity,summary,1,Budget::unlimited()); CHECK(none.values.empty()); CHECK(none.summary.exact_evaluations==0); CHECK(one.values.size()<=1); CHECK(one.summary.exact_evaluations==one.values.size()); }
TEST(kevin_filter_invalid_modulus_and_empty_mask_never_request_exact){ using namespace bruce::lhme; auto engine=Engine::create({}); CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>())); KevinMemoryStore store{engine}; GridSpec grid{{.id="bruce.invalid-modulus",.version=1},{.id="bruce.power_offset",.version=1},encode_power_offset_parameters(2,100),1}; CHECK(store.register_grid(grid)); const auto parent=*GridAddress::from_steps({}); const auto invalid=modular_equals_tile(store,grid.identity,parent,0,0); CHECK(invalid.bits()==0); const auto empty=filter_tile(store,grid.identity,parent,0,0,0,100); CHECK(empty.candidates.empty()); const auto exact=evaluate_survivors_exact(store,grid.identity,empty,64,Budget::unlimited()); CHECK(exact.values.empty()); CHECK(exact.summary.exact_evaluations==0); CHECK(store.inspect().computed_exact_evaluation_count==0); }
TEST(kevin_filter_same_tile_and_parameters_are_deterministic){ using namespace bruce::lhme; auto engine=Engine::create({}); CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>())); KevinMemoryStore store{engine}; GridSpec grid{{.id="bruce.repeatable",.version=1},{.id="bruce.power_offset",.version=1},encode_power_offset_parameters(2,100),1}; CHECK(store.register_grid(grid)); const auto parent=*GridAddress::from_steps({}); const auto first=filter_tile(store,grid.identity,parent,17,11,7,17); const auto second=filter_tile(store,grid.identity,parent,17,11,7,17); CHECK(first.modular_survivors==second.modular_survivors); CHECK(first.metric_survivors==second.metric_survivors); CHECK(first.final_survivors==second.final_survivors); CHECK(first.candidates==second.candidates); CHECK(store.inspect().computed_exact_evaluation_count==0); }
TEST(kevin_filter_reduces_multiple_tiles_before_one_global_exact_limit){ using namespace bruce::lhme; auto engine=Engine::create({}); CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>())); KevinMemoryStore store{engine}; GridSpec grid{{.id="bruce.reduction",.version=1},{.id="bruce.power_offset",.version=1},encode_power_offset_parameters(2,100),2}; CHECK(store.register_grid(grid)); const auto root=*GridAddress::from_steps({}); const std::vector<GridStep> nested_steps{{.x=1,.y=0}}; const auto nested=*GridAddress::from_steps(nested_steps); const std::vector<GridAddress> parents{root,nested}; const auto summary=filter_tiles(store,grid.identity,parents,17,11,7,17); CHECK(summary.tile_count==2); CHECK(summary.checked==128); CHECK(summary.final_survivors==summary.candidates.size()); CHECK(store.inspect().computed_exact_evaluation_count==0); const auto exact=evaluate_reduced_exact(store,grid.identity,summary,1,Budget::unlimited()); CHECK(exact.values.size()<=1); CHECK(exact.summary.exact_evaluations==exact.values.size()); }
TEST(kevin_filter_cpu_dispatcher_matches_the_stable_sequential_reduction){ using namespace bruce::lhme; auto engine=Engine::create({.cache_max_entries=128,.cache_max_bytes=65536}); CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>())); KevinMemoryStore store{engine}; GridSpec grid{{.id="bruce.cpu-reduction",.version=1},{.id="bruce.power_offset",.version=1},encode_power_offset_parameters(2,100),2}; CHECK(store.register_grid(grid)); const auto root=*GridAddress::from_steps({}); const std::vector<GridStep> first_steps{{.x=1,.y=0}}; const std::vector<GridStep> second_steps{{.x=2,.y=0}}; const std::vector<GridAddress> parents{root,*GridAddress::from_steps(first_steps),*GridAddress::from_steps(second_steps)}; const auto sequential=filter_tiles(store,grid.identity,parents,17,11,7,17); const auto parallel=filter_tiles_cpu(store,grid.identity,parents,17,11,7,17,2); CHECK(parallel.tile_count==sequential.tile_count); CHECK(parallel.checked==sequential.checked); CHECK(parallel.modular_survivors==sequential.modular_survivors); CHECK(parallel.metric_survivors==sequential.metric_survivors); CHECK(parallel.final_survivors==sequential.final_survivors); CHECK(parallel.candidates==sequential.candidates); CHECK(store.inspect().computed_exact_evaluation_count==0); }
TEST(kevin_filter_cpu_dispatcher_rejects_zero_workers){ using namespace bruce::lhme; auto engine=Engine::create({}); CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>())); KevinMemoryStore store{engine}; GridSpec grid{{.id="bruce.cpu-invalid",.version=1},{.id="bruce.power_offset",.version=1},encode_power_offset_parameters(2,100),1}; CHECK(store.register_grid(grid)); const std::vector<GridAddress> parents{*GridAddress::from_steps({})}; const auto result=filter_tiles_cpu(store,grid.identity,parents,17,11,7,17,0); CHECK(result.status==Status::InvalidArgument); CHECK(result.checked==0); }
TEST(kevin_persistent_cpu_executor_reuses_workers_for_deterministic_reductions){ using namespace bruce::lhme; auto engine=Engine::create({.cache_max_entries=128,.cache_max_bytes=65536}); CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>())); KevinMemoryStore store{engine}; GridSpec grid{{.id="bruce.persistent-cpu",.version=1},{.id="bruce.power_offset",.version=1},encode_power_offset_parameters(2,100),2}; CHECK(store.register_grid(grid)); const auto root=*GridAddress::from_steps({}); const std::vector<GridStep> nested_steps{{.x=1,.y=0}}; const std::vector<GridAddress> parents{root,*GridAddress::from_steps(nested_steps)}; KevinTileCpuExecutor executor{2}; const auto first=executor.filter(store,grid.identity,parents,17,11,7,17); const auto second=executor.filter(store,grid.identity,parents,17,11,7,17); CHECK(first.status==Status::Confirmed); CHECK(second.status==Status::Confirmed); CHECK(first.checked==128); CHECK(second.checked==first.checked); CHECK(second.final_survivors==first.final_survivors); CHECK(second.candidates==first.candidates); CHECK(store.inspect().computed_exact_evaluation_count==0); }
