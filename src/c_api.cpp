#include "axiom/lhme/axiom_lhme_c_api.h"

#include <limits>
#include <memory>
#include <optional>
#include <string>

#include "axiom/lhme/engine.hpp"

struct axiom_lhme_engine {
  explicit axiom_lhme_engine(axiom::lhme::EngineConfig config)
      : implementation(axiom::lhme::Engine::create(config)) {}
  axiom::lhme::Engine implementation;
};

struct axiom_lhme_formula { axiom::lhme::FormulaSpec specification; };

struct axiom_lhme_result {
  axiom::lhme::EvaluationResult evaluation;
  std::string exact_decimal;
  std::string inspection;
};

namespace {

std::optional<axiom::lhme::EvaluationMode> to_cpp_mode(axiom_lhme_mode mode) {
  switch (mode) {
    case AXIOM_LHME_MODE_SYMBOLIC: return axiom::lhme::EvaluationMode::Symbolic;
    case AXIOM_LHME_MODE_METRIC: return axiom::lhme::EvaluationMode::Metric;
    case AXIOM_LHME_MODE_MODULAR: return axiom::lhme::EvaluationMode::Modular;
    case AXIOM_LHME_MODE_EXACT: return axiom::lhme::EvaluationMode::Exact;
  }
  return std::nullopt;
}

axiom_lhme_status to_c_status(axiom::lhme::Status status) {
  switch (status) {
    case axiom::lhme::Status::Confirmed: return AXIOM_LHME_STATUS_CONFIRMED;
    case axiom::lhme::Status::Rejected: return AXIOM_LHME_STATUS_REJECTED;
    case axiom::lhme::Status::BudgetExceeded: return AXIOM_LHME_STATUS_BUDGET_EXCEEDED;
    case axiom::lhme::Status::Cancelled: return AXIOM_LHME_STATUS_CANCELLED;
    case axiom::lhme::Status::InvalidArgument: return AXIOM_LHME_STATUS_INVALID_ARGUMENT;
    case axiom::lhme::Status::UnsupportedMode: return AXIOM_LHME_STATUS_UNSUPPORTED_MODE;
    case axiom::lhme::Status::InternalError: return AXIOM_LHME_STATUS_INTERNAL_ERROR;
  }
  return AXIOM_LHME_STATUS_INTERNAL_ERROR;
}

axiom_lhme_result* make_result(axiom::lhme::EvaluationResult evaluation) {
  auto result = std::make_unique<axiom_lhme_result>();
  result->evaluation = std::move(evaluation);
  if (result->evaluation.integer.has_value()) {
    result->exact_decimal = result->evaluation.integer->convert_to<std::string>();
  }
  result->inspection = std::to_string(static_cast<int>(result->evaluation.status));
  return result.release();
}

axiom_lhme_result* internal_error_result() noexcept {
  try {
    return make_result({.status = axiom::lhme::Status::InternalError});
  } catch (...) {
    return nullptr;
  }
}

}  // namespace

extern "C" axiom_lhme_engine* axiom_lhme_engine_create(const axiom_lhme_engine_config* config) {
  try {
    const axiom_lhme_engine_config safe = config == nullptr
                                        ? axiom_lhme_engine_config{0, 0, 1, 1}
                                        : *config;
    return new axiom_lhme_engine({.cache_max_entries = safe.cache_max_entries,
                            .cache_max_bytes = safe.cache_max_bytes,
                            .worker_count = safe.worker_count,
                            .tile_size = safe.tile_size});
  } catch (...) {
    return nullptr;
  }
}

extern "C" void axiom_lhme_engine_destroy(axiom_lhme_engine* engine) { delete engine; }

extern "C" axiom_lhme_formula* axiom_lhme_formula_register(
    axiom_lhme_engine* engine, const axiom_lhme_power_offset_spec* spec) {
  if (engine == nullptr || spec == nullptr || spec->base_decimal == nullptr ||
      spec->scale_decimal == nullptr) {
    return nullptr;
  }
  try {
    const auto registered = engine->implementation.register_formula(
        axiom::lhme::make_power_offset_spec(axiom::lhme::BigInt(spec->base_decimal),
                                            axiom::lhme::BigInt(spec->scale_decimal),
                                            spec->version));
    return new axiom_lhme_formula{.specification = registered};
  } catch (...) {
    return nullptr;
  }
}

extern "C" void axiom_lhme_formula_destroy(axiom_lhme_formula* formula) { delete formula; }

extern "C" axiom_lhme_result* axiom_lhme_evaluate(
    axiom_lhme_engine* engine, const axiom_lhme_formula* formula,
    const axiom_lhme_evaluation_request* request) {
  if (engine == nullptr || formula == nullptr || request == nullptr) {
    return internal_error_result();
  }
  try {
    const auto mode = to_cpp_mode(request->mode);
    if (!mode.has_value()) {
      return make_result({.status = axiom::lhme::Status::InvalidArgument});
    }
    axiom::lhme::EvaluationRequest cpp_request{
        .spec = formula->specification,
        .address = {.index = request->index},
        .mode = *mode,
        .budget = {.max_bit_length = request->max_bit_length},
        .modulus = request->modulus_decimal == nullptr
                       ? std::nullopt
                       : std::optional<axiom::lhme::BigInt>(
                             axiom::lhme::BigInt(request->modulus_decimal))};
    return make_result(engine->implementation.evaluate(cpp_request));
  } catch (...) {
    return internal_error_result();
  }
}

extern "C" axiom_lhme_status axiom_lhme_result_status(const axiom_lhme_result* result) {
  return result == nullptr ? AXIOM_LHME_STATUS_INTERNAL_ERROR
                           : to_c_status(result->evaluation.status);
}

extern "C" const char* axiom_lhme_result_exact_decimal(const axiom_lhme_result* result) {
  if (result == nullptr || !result->evaluation.integer.has_value()) return nullptr;
  return result->exact_decimal.c_str();
}

extern "C" const char* axiom_lhme_result_inspect(const axiom_lhme_result* result) {
  return result == nullptr ? nullptr : result->inspection.c_str();
}

extern "C" void axiom_lhme_result_destroy(axiom_lhme_result* result) { delete result; }

extern "C" void axiom_lhme_cancel(axiom_lhme_engine* engine) {
  if (engine != nullptr) engine->implementation.cancel();
}
