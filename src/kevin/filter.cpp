#include "axiom/lhme/kevin_filter.hpp"

#include "axiom/lhme/trace.hpp"

#include <algorithm>
#include <future>
#include <string>
namespace axiom::lhme {
namespace {

Budget stricter_budget(Budget first, const Budget& second) {
  first.max_bit_length = std::min(first.max_bit_length, second.max_bit_length);
  first.max_operations = std::min(first.max_operations, second.max_operations);
  first.max_temporary_bytes =
      std::min(first.max_temporary_bytes, second.max_temporary_bytes);
  first.max_trace_bytes = std::min(first.max_trace_bytes, second.max_trace_bytes);
  first.max_candidate_count =
      std::min(first.max_candidate_count, second.max_candidate_count);
  return first;
}

std::string identity_text(const GridIdentity& identity) {
  return identity.id + "@" + std::to_string(identity.version);
}

std::string identity_text(const ProjectionIdentity& identity) {
  return identity.id + "@" + std::to_string(identity.version);
}

void append_domain_trace(EvaluationResult& result, GateScope scope,
                         std::string reason) {
  std::vector<GateEvent> events;
  if (result.trace != nullptr) events = result.trace->events;
  events.push_back({.scope = scope, .status = result.status,
                    .reason = std::move(reason)});
  result.trace = make_trace(std::move(events));
}

EvaluationResult invalid_domain_result(EvaluationMode mode, std::string reason) {
  EvaluationResult result;
  result.status = Status::InvalidArgument;
  result.mode = mode;
  append_domain_trace(result, GateScope::Branch, std::move(reason));
  return result;
}

std::optional<GridAddress> child_address(const GridAddress& parent,
                                         std::uint8_t x, std::uint8_t y) {
  auto steps = parent.steps();
  steps.push_back({x, y});
  return GridAddress::from_steps(steps);
}

bool valid_domain_context(const DomainEvaluationContext& context) {
  return valid_grid_domain_spec(context.domain) && context.projection != nullptr &&
         valid_projection_identity(context.projection->identity());
}

}  // namespace

EvaluationResult evaluate_domain_address(const DomainEvaluationContext& context,
                                         const GridAddress& address,
                                         EvaluationMode mode, Budget budget,
                                         std::optional<BigInt> modulus) {
  const auto domain = identity_text(context.domain.identity);
  if (!valid_grid_domain_spec(context.domain)) {
    return invalid_domain_result(mode, "domain=" + domain + ";domain=invalid");
  }
  if (context.projection == nullptr) {
    return invalid_domain_result(mode, "domain=" + domain + ";projection=missing");
  }
  const auto projection = context.projection->identity();
  if (!valid_projection_identity(projection)) {
    return invalid_domain_result(mode,
                                 "domain=" + domain + ";projection=invalid");
  }
  const auto projection_text = identity_text(projection);
  if (address.depth() > context.domain.max_depth) {
    return invalid_domain_result(mode, "domain=" + domain + ";projection=" +
                                           projection_text + ";address=out-of-range");
  }
  const auto formula_address = context.projection->project(address);
  if (!formula_address.has_value()) {
    return invalid_domain_result(mode, "domain=" + domain + ";projection=" +
                                           projection_text + ";formula=unmapped");
  }
  auto result = context.evaluator.evaluate(
      *formula_address, mode, stricter_budget(budget, context.domain.default_budget),
      std::move(modulus));
  append_domain_trace(result, GateScope::Projection,
                      "domain=" + domain + ";projection=" + projection_text +
                          ";formula=" + std::to_string(formula_address->index));
  return result;
}

CandidateMask64 modular_equals_tile(const DomainEvaluationContext& context,
                                    const GridAddress& parent,
                                    const BigInt& modulus,
                                    const BigInt& residue) {
  if (modulus <= 0) return {};
  std::uint64_t mask = 0;
  for (std::uint8_t y = 0; y < 8; ++y) {
    for (std::uint8_t x = 0; x < 8; ++x) {
      const auto child = child_address(parent, x, y);
      if (!child.has_value()) continue;
      const auto value = evaluate_domain_address(context, *child,
                                                 EvaluationMode::Modular,
                                                 Budget::unlimited(), modulus);
      if (value.status == Status::Confirmed && value.integer == residue) {
        mask |= 1ULL << grid_tile_bit(x, y);
      }
    }
  }
  return {mask};
}

CandidateMask64 metric_bit_length_between_tile(
    const DomainEvaluationContext& context, const GridAddress& parent,
    std::uint64_t minimum, std::uint64_t maximum) {
  if (minimum > maximum) return {};
  std::uint64_t mask = 0;
  for (std::uint8_t y = 0; y < 8; ++y) {
    for (std::uint8_t x = 0; x < 8; ++x) {
      const auto child = child_address(parent, x, y);
      if (!child.has_value()) continue;
      const auto value = evaluate_domain_address(context, *child,
                                                 EvaluationMode::Metric,
                                                 Budget::unlimited());
      if (value.status == Status::Confirmed && value.metric_bit_length &&
          *value.metric_bit_length >= minimum &&
          *value.metric_bit_length <= maximum) {
        mask |= 1ULL << grid_tile_bit(x, y);
      }
    }
  }
  return {mask};
}

TileSummary filter_tile(const DomainEvaluationContext& context,
                        const GridAddress& parent, const BigInt& modulus,
                        const BigInt& residue, std::uint64_t minimum,
                        std::uint64_t maximum) {
  if (!valid_domain_context(context) || parent.depth() >= context.domain.max_depth ||
      modulus <= 0 || minimum > maximum) {
    return {.status = Status::InvalidArgument, .checked = 0};
  }

  const auto modular = modular_equals_tile(context, parent, modulus, residue);
  const auto metric = metric_bit_length_between_tile(context, parent, minimum, maximum);
  TileSummary output{.modular_survivors = modular.popcount(),
                     .metric_survivors = metric.popcount()};
  const auto matching_bits = (modular & metric).bits();
  output.final_survivors = std::popcount(matching_bits);
  for (std::uint8_t y = 0; y < 8; ++y) {
    for (std::uint8_t x = 0; x < 8; ++x) {
      if ((matching_bits & (1ULL << grid_tile_bit(x, y))) != 0) {
        output.candidates.push_back(*child_address(parent, x, y));
      }
    }
  }
  return output;
}

DomainExactTileResult evaluate_survivors_exact(
    const DomainEvaluationContext& context, TileSummary summary,
    std::uint64_t limit, Budget budget) {
  DomainExactTileResult output{.summary = std::move(summary)};
  if (!valid_domain_context(context)) {
    output.summary.status = Status::InvalidArgument;
    return output;
  }
  for (const auto& address : output.summary.candidates) {
    if (output.values.size() >= limit) break;
    auto value = evaluate_domain_address(context, address, EvaluationMode::Exact,
                                         budget);
    if (value.status != Status::Confirmed) output.summary.status = value.status;
    output.values.push_back(std::move(value));
    ++output.summary.exact_evaluations;
  }
  return output;
}

CandidateMask64 modular_equals_tile(KevinMemoryStore& store,const GridIdentity& grid,const GridAddress& parent,const BigInt& modulus,const BigInt& residue){
  if(modulus<=0) return {};
  std::uint64_t mask=0;
  for(std::uint8_t y=0;y<8;++y) for(std::uint8_t x=0;x<8;++x){ auto steps=parent.steps(); steps.push_back({x,y}); const auto child=GridAddress::from_steps(steps); if(!child) continue; const auto value=store.get(grid,*child,EvaluationMode::Modular,Budget::unlimited(),modulus); if(value.result.status==Status::Confirmed && value.result.integer==residue) mask|=1ULL<<grid_tile_bit(x,y); }
  return {mask};
}
CandidateMask64 metric_bit_length_between_tile(KevinMemoryStore& store,const GridIdentity& grid,const GridAddress& parent,std::uint64_t minimum,std::uint64_t maximum){ std::uint64_t mask=0; if(minimum>maximum)return {}; for(std::uint8_t y=0;y<8;++y)for(std::uint8_t x=0;x<8;++x){auto steps=parent.steps();steps.push_back({x,y});const auto child=GridAddress::from_steps(steps);if(!child)continue;const auto value=store.get(grid,*child,EvaluationMode::Metric,Budget::unlimited());if(value.result.status==Status::Confirmed&&value.result.metric_bit_length&&*value.result.metric_bit_length>=minimum&&*value.result.metric_bit_length<=maximum)mask|=1ULL<<grid_tile_bit(x,y);}return {mask}; }
TileSummary filter_tile(KevinMemoryStore& store,const GridIdentity& grid,const GridAddress& parent,const BigInt& modulus,const BigInt& residue,std::uint64_t minimum,std::uint64_t maximum){auto a=modular_equals_tile(store,grid,parent,modulus,residue);auto b=metric_bit_length_between_tile(store,grid,parent,minimum,maximum);TileSummary out{.modular_survivors=a.popcount(),.metric_survivors=b.popcount()};auto bits=(a&b).bits();out.final_survivors=std::popcount(bits);for(std::uint8_t y=0;y<8;++y)for(std::uint8_t x=0;x<8;++x)if(bits&(1ULL<<grid_tile_bit(x,y))){auto s=parent.steps();s.push_back({x,y});out.candidates.push_back(*GridAddress::from_steps(s));}return out;}
ExactTileResult evaluate_survivors_exact(KevinMemoryStore& store,const GridIdentity& grid,TileSummary summary,std::uint64_t limit,Budget budget){ExactTileResult out{.summary=std::move(summary)};for(const auto& address:out.summary.candidates){if(out.values.size()>=limit)break;out.values.push_back(store.get(grid,address,EvaluationMode::Exact,budget));++out.summary.exact_evaluations;}return out;}
TileReductionSummary filter_tiles(KevinMemoryStore& store,const GridIdentity& grid,std::span<const GridAddress> parents,const BigInt& modulus,const BigInt& residue,std::uint64_t minimum,std::uint64_t maximum){ TileReductionSummary out{.tile_count=parents.size()}; for(const auto& parent:parents){const auto tile=filter_tile(store,grid,parent,modulus,residue,minimum,maximum);out.checked+=tile.checked;out.modular_survivors+=tile.modular_survivors;out.metric_survivors+=tile.metric_survivors;out.final_survivors+=tile.final_survivors;out.candidates.insert(out.candidates.end(),tile.candidates.begin(),tile.candidates.end());}return out;}
TileReductionSummary filter_tiles_cpu(KevinMemoryStore& store,const GridIdentity& grid,std::span<const GridAddress> parents,const BigInt& modulus,const BigInt& residue,std::uint64_t minimum,std::uint64_t maximum,std::uint32_t worker_count){ TileReductionSummary out{.tile_count=parents.size()}; if(worker_count==0){out.status=Status::InvalidArgument;return out;} for(std::size_t next=0;next<parents.size();){std::vector<std::future<TileSummary>> batch; const auto count=std::min<std::size_t>(worker_count,parents.size()-next);batch.reserve(count);for(std::size_t slot=0;slot<count;++slot){const auto parent=parents[next++];batch.push_back(std::async(std::launch::async,[&store,&grid,&modulus,&residue,minimum,maximum,parent]{return filter_tile(store,grid,parent,modulus,residue,minimum,maximum);}));}for(auto& future:batch){const auto tile=future.get();out.checked+=tile.checked;out.modular_survivors+=tile.modular_survivors;out.metric_survivors+=tile.metric_survivors;out.final_survivors+=tile.final_survivors;out.candidates.insert(out.candidates.end(),tile.candidates.begin(),tile.candidates.end());}}return out;}
ExactTileReductionResult evaluate_reduced_exact(KevinMemoryStore& store,const GridIdentity& grid,TileReductionSummary summary,std::uint64_t limit,Budget budget){ ExactTileReductionResult out{.summary=std::move(summary)}; for(const auto& address:out.summary.candidates){if(out.values.size()>=limit)break;out.values.push_back(store.get(grid,address,EvaluationMode::Exact,budget));++out.summary.exact_evaluations;}return out;}
}  // namespace axiom::lhme
