#include "bruce/lhme/c_api.h"

#include <limits>
#include <memory>
#include <optional>
#include <string>

#include "bruce/lhme/engine.hpp"

struct lhme_engine {
  explicit lhme_engine(bruce::lhme::EngineConfig config)
      : implementation(bruce::lhme::Engine::create(config)) {}
  bruce::lhme::Engine implementation;
};

struct lhme_formula { bruce::lhme::FormulaSpec specification; };

struct lhme_result {
  bruce::lhme::EvaluationResult evaluation;
  std::string exact_decimal;
  std::string inspection;
};

namespace {

std::optional<bruce::lhme::EvaluationMode> to_cpp_mode(lhme_mode mode) {
  switch (mode) {
    case LHME_MODE_SYMBOLIC: return bruce::lhme::EvaluationMode::Symbolic;
    case LHME_MODE_METRIC: return bruce::lhme::EvaluationMode::Metric;
    case LHME_MODE_MODULAR: return bruce::lhme::EvaluationMode::Modular;
    case LHME_MODE_EXACT: return bruce::lhme::EvaluationMode::Exact;
  }
  return std::nullopt;
}

lhme_status to_c_status(bruce::lhme::Status status) {
  switch (status) {
    case bruce::lhme::Status::Confirmed: return LHME_STATUS_CONFIRMED;
    case bruce::lhme::Status::Rejected: return LHME_STATUS_REJECTED;
    case bruce::lhme::Status::BudgetExceeded: return LHME_STATUS_BUDGET_EXCEEDED;
    case bruce::lhme::Status::Cancelled: return LHME_STATUS_CANCELLED;
    case bruce::lhme::Status::InvalidArgument: return LHME_STATUS_INVALID_ARGUMENT;
    case bruce::lhme::Status::UnsupportedMode: return LHME_STATUS_UNSUPPORTED_MODE;
    case bruce::lhme::Status::InternalError: return LHME_STATUS_INTERNAL_ERROR;
  }
  return LHME_STATUS_INTERNAL_ERROR;
}

lhme_result* make_result(bruce::lhme::EvaluationResult evaluation) {
  auto result = std::make_unique<lhme_result>();
  result->evaluation = std::move(evaluation);
  if (result->evaluation.integer.has_value()) {
    result->exact_decimal = result->evaluation.integer->convert_to<std::string>();
  }
  result->inspection = std::to_string(static_cast<int>(result->evaluation.status));
  return result.release();
}

lhme_result* internal_error_result() noexcept {
  try {
    return make_result({.status = bruce::lhme::Status::InternalError});
  } catch (...) {
    return nullptr;
  }
}

}  // namespace

extern "C" lhme_engine* lhme_engine_create(const lhme_engine_config* config) {
  try {
    const lhme_engine_config safe = config == nullptr
                                        ? lhme_engine_config{0, 0, 1, 1}
                                        : *config;
    return new lhme_engine({.cache_max_entries = safe.cache_max_entries,
                            .cache_max_bytes = safe.cache_max_bytes,
                            .worker_count = safe.worker_count,
                            .tile_size = safe.tile_size});
  } catch (...) {
    return nullptr;
  }
}

extern "C" void lhme_engine_destroy(lhme_engine* engine) { delete engine; }

extern "C" lhme_formula* lhme_formula_register(
    lhme_engine* engine, const lhme_power_offset_spec* spec) {
  if (engine == nullptr || spec == nullptr || spec->base_decimal == nullptr ||
      spec->scale_decimal == nullptr) {
    return nullptr;
  }
  try {
    const auto registered = engine->implementation.register_formula(
        bruce::lhme::make_power_offset_spec(bruce::lhme::BigInt(spec->base_decimal),
                                            bruce::lhme::BigInt(spec->scale_decimal),
                                            spec->version));
    return new lhme_formula{.specification = registered};
  } catch (...) {
    return nullptr;
  }
}

extern "C" void lhme_formula_destroy(lhme_formula* formula) { delete formula; }

extern "C" lhme_result* lhme_evaluate(
    lhme_engine* engine, const lhme_formula* formula,
    const lhme_evaluation_request* request) {
  if (engine == nullptr || formula == nullptr || request == nullptr) {
    return internal_error_result();
  }
  try {
    const auto mode = to_cpp_mode(request->mode);
    if (!mode.has_value()) {
      return make_result({.status = bruce::lhme::Status::InvalidArgument});
    }
    bruce::lhme::EvaluationRequest cpp_request{
        .spec = formula->specification,
        .address = {.index = request->index},
        .mode = *mode,
        .budget = {.max_bit_length = request->max_bit_length},
        .modulus = request->modulus_decimal == nullptr
                       ? std::nullopt
                       : std::optional<bruce::lhme::BigInt>(
                             bruce::lhme::BigInt(request->modulus_decimal))};
    return make_result(engine->implementation.evaluate(cpp_request));
  } catch (...) {
    return internal_error_result();
  }
}

extern "C" lhme_status lhme_result_status(const lhme_result* result) {
  return result == nullptr ? LHME_STATUS_INTERNAL_ERROR
                           : to_c_status(result->evaluation.status);
}

extern "C" const char* lhme_result_exact_decimal(const lhme_result* result) {
  if (result == nullptr || !result->evaluation.integer.has_value()) return nullptr;
  return result->exact_decimal.c_str();
}

extern "C" const char* lhme_result_inspect(const lhme_result* result) {
  return result == nullptr ? nullptr : result->inspection.c_str();
}

extern "C" void lhme_result_destroy(lhme_result* result) { delete result; }

extern "C" void lhme_cancel(lhme_engine* engine) {
  if (engine != nullptr) engine->implementation.cancel();
}
