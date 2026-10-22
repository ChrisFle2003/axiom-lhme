#include "axiom/lhme/engine.hpp"

#include "axiom/lhme/trace.hpp"

#include <mutex>

namespace axiom::lhme {
namespace {

EvaluationResult cancelled_result(EvaluationMode mode) {
  EvaluationResult result;
  result.status = Status::Cancelled;
  result.mode = mode;
  result.trace = make_trace({{.scope = GateScope::Run,
                              .status = Status::Cancelled,
                              .reason = "engine cancellation requested"}});
  return result;
}

std::string technique_registry_key(const TechniqueIdentity& identity) {
  return std::to_string(identity.id.size()) + ":" + identity.id + ":" +
         std::to_string(identity.version);
}

EvaluationResult technique_rejection(EvaluationMode mode, const char* reason) {
  EvaluationResult result;
  result.status = Status::InvalidArgument;
  result.mode = mode;
  result.trace = make_trace({{.scope = GateScope::Branch,
                              .status = result.status,
                              .reason = reason}});
  return result;
}

EvaluationResult technique_internal_error(EvaluationMode mode) {
  EvaluationResult result;
  result.status = Status::InternalError;
  result.mode = mode;
  result.trace = make_trace({{.scope = GateScope::Branch,
                              .status = result.status,
                              .reason = "technique kernel threw an exception"}});
  return result;
}

void append_cache_event(EvaluationResult& result, const char* reason) {
  std::vector<GateEvent> events;
  if (result.trace != nullptr) {
    events = result.trace->events;
  }
  events.push_back({.scope = GateScope::Cache,
                    .status = result.status,
                    .reason = reason});
  result.trace = make_trace(std::move(events));
}

void append_engine_cancellation(EvaluationResult& result) {
  std::vector<GateEvent> events;
  if (result.trace != nullptr) {
    events = result.trace->events;
  }
  result.status = Status::Cancelled;
  result.integer.reset();
  result.metric_bit_length.reset();
  result.symbolic_expression.reset();
  events.push_back({.scope = GateScope::Run,
                    .status = result.status,
                    .reason = "engine cancellation requested"});
  result.trace = make_trace(std::move(events));
}

std::uint64_t bit_length(const BigInt& value) {
  if (value == 0) return 0;
  const auto magnitude = value < 0 ? -value : value;
  return static_cast<std::uint64_t>(boost::multiprecision::msb(magnitude)) + 1U;
}

std::uint64_t estimated_operations(std::int64_t index) {
  if (index <= 0) return 0;
  std::uint64_t value = static_cast<std::uint64_t>(index);
  std::uint64_t bits = 0;
  while (value > 0) {
    ++bits;
    value >>= 1U;
  }
  return bits * 2U;
}

std::uint64_t bits_to_bytes(std::uint64_t bits) {
  return bits / 8U + (bits % 8U == 0 ? 0U : 1U);
}

template <typename Request>
bool cache_budget_rejects(const Request& request, std::uint64_t value_bit_length) {
  return value_bit_length > request.budget.max_bit_length ||
         estimated_operations(request.address.index) > request.budget.max_operations ||
         bits_to_bytes(value_bit_length) > request.budget.max_temporary_bytes;
}

template <typename Request>
EvaluationResult value_exceeds_budget(const Request& request,
                                      std::uint64_t value_bit_length) {
  EvaluationResult result;
  result.status = Status::BudgetExceeded;
  result.mode = request.mode;
  result.metric_bit_length = value_bit_length;
  result.trace = make_trace({{.scope = GateScope::Publish,
                              .status = result.status,
                              .reason = "result exceeded caller resource budget"}});
  return result;
}

std::optional<std::uint64_t> result_bit_length(const EvaluationResult& result) {
  if (result.integer.has_value()) return bit_length(*result.integer);
  return result.metric_bit_length;
}

}  // namespace

Engine::Engine(EngineConfig config)
    : config_(config),
      cache_({.max_entries = config.cache_max_entries,
              .max_bytes = config.cache_max_bytes}),
      cancellation_signal_(std::make_shared<std::atomic_bool>(false)),
      registry_mutex_(std::make_shared<std::shared_mutex>()) {}

Engine Engine::create(EngineConfig config) { return Engine(config); }

FormulaSpec Engine::register_formula(FormulaSpec spec) {
  const std::unique_lock lock(*registry_mutex_);
  formulas_.insert_or_assign(canonical_formula_identity(spec), spec);
  return spec;
}

bool Engine::register_technique(std::shared_ptr<const TechniqueKernel> kernel) {
  try {
    if (kernel == nullptr) return false;
    const auto identity = kernel->identity();
    if (!valid_technique_identity(identity)) return false;
    const auto key = technique_registry_key(identity);
    const std::unique_lock lock(*registry_mutex_);
    if (techniques_.contains(key)) return false;
    techniques_.emplace(key, std::move(kernel));
    return true;
  } catch (...) {
    return false;
  }
}

bool Engine::has_technique(const TechniqueIdentity& identity) const {
  const std::shared_lock lock(*registry_mutex_);
  return valid_technique_identity(identity) &&
         techniques_.contains(technique_registry_key(identity));
}

bool Engine::is_registered(const FormulaSpec& spec) const {
  const std::shared_lock lock(*registry_mutex_);
  const auto it = formulas_.find(canonical_formula_identity(spec));
  return it != formulas_.end() && it->second.version == spec.version &&
         it->second.operation == spec.operation && it->second.base == spec.base &&
         it->second.scale == spec.scale &&
         it->second.nonnegative_index == spec.nonnegative_index;
}

EvaluationResult Engine::evaluate(const EvaluationRequest& request) {
  if (cancellation_signal_->load() || request.cancellation_requested) {
    return cancelled_result(request.mode);
  }
  if (!is_registered(request.spec)) {
    EvaluationResult result;
    result.status = Status::InvalidArgument;
    result.mode = request.mode;
    result.trace = make_trace({{.scope = GateScope::Branch,
                                .status = result.status,
                                .reason = "formula is not registered"}});
    return result;
  }

  const bool cacheable = request.mode == EvaluationMode::Exact ||
                         request.mode == EvaluationMode::Metric;
  CacheKey key{};
  if (cacheable) {
    key = request.mode == EvaluationMode::Exact
              ? CacheKey::exact(request.spec, request.address.index)
              : CacheKey::metric(request.spec, request.address.index);
    if (const auto cached = cache_.lookup(key); cached.has_value()) {
      auto result = *cached;
      if (request.mode == EvaluationMode::Exact && result.integer.has_value() &&
          cache_budget_rejects(request, bit_length(*result.integer))) {
        auto rejection = value_exceeds_budget(request, bit_length(*result.integer));
        append_cache_event(rejection, "cache rejected by caller budget");
        return rejection;
      }
      append_cache_event(result, "cache hit");
      return result;
    }
  }

  auto effective_request = request;
  effective_request.cancellation_signal = cancellation_signal_;
  EvaluationResult result = axiom::lhme::evaluate(effective_request);
  if (cacheable) {
    append_cache_event(result, "cache miss");
    cache_.admit(key, result);
  }
  return result;
}

EvaluationResult Engine::evaluate_technique(const TechniqueRequest& request) {
  if (cancellation_signal_->load()) {
    return cancelled_result(request.mode);
  }
  if (!valid_technique_identity(request.identity)) {
    return technique_rejection(request.mode, "technique identity is invalid");
  }
  if (request.parameters.empty() || request.parameters.size() > 4096) {
    return technique_rejection(request.mode, "technique parameters are invalid");
  }
  std::shared_ptr<const TechniqueKernel> kernel;
  {
    const std::shared_lock lock(*registry_mutex_);
    const auto found = techniques_.find(technique_registry_key(request.identity));
    if (found == techniques_.end()) {
      return technique_rejection(request.mode, "technique is not registered");
    }
    kernel = found->second;
  }

  auto effective_request = request;
  effective_request.cancellation_signal = cancellation_signal_;
  try {
    const bool cacheable = request.mode == EvaluationMode::Exact ||
                           request.mode == EvaluationMode::Metric;
    CacheKey key{};
    if (cacheable) {
      key = CacheKey::technique(request.identity, request.parameters,
                                request.address.index, request.mode);
      if (const auto cached = cache_.lookup(key); cached.has_value()) {
        auto result = *cached;
        if (request.mode == EvaluationMode::Exact && result.integer.has_value() &&
            cache_budget_rejects(request, bit_length(*result.integer))) {
          auto rejection = value_exceeds_budget(request, bit_length(*result.integer));
          append_cache_event(rejection, "cache rejected by caller budget");
          return rejection;
        }
        if (cancellation_signal_->load()) {
          append_engine_cancellation(result);
          return result;
        }
        append_cache_event(result, "cache hit");
        return result;
      }
    }

    EvaluationResult result = kernel->evaluate(effective_request);
    if (cancellation_signal_->load()) {
      append_engine_cancellation(result);
      return result;
    }
    if (result.status == Status::Confirmed) {
      if (const auto bits = result_bit_length(result);
          bits.has_value() && cache_budget_rejects(request, *bits)) {
        return value_exceeds_budget(request, *bits);
      }
    }
    if (cacheable) {
      append_cache_event(result, "cache miss");
      cache_.admit(key, result);
    }
    return result;
  } catch (...) {
    return technique_internal_error(request.mode);
  }
}

RangeResult Engine::evaluate_range(const EvaluationRequest& request,
                                   AddressRange range) {
  if (cancellation_signal_->load()) {
    return {.status = Status::Cancelled};
  }
  if (!is_registered(request.spec)) {
    return {.status = Status::InvalidArgument};
  }
  auto effective_request = request;
  effective_request.cancellation_signal = cancellation_signal_;
  return axiom::lhme::evaluate_range(
      effective_request, range,
      {.worker_count = config_.worker_count, .tile_size = config_.tile_size});
}

CacheStatistics Engine::inspect() const { return cache_.statistics(); }

void Engine::cancel() noexcept {
  cancellation_signal_->store(true);
  cancellation_signal_->notify_all();
}

}  // namespace axiom::lhme
