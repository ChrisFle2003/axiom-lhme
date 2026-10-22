#include <atomic>
#include <memory>

#include "axiom/lhme/engine.hpp"
#include "axiom/lhme/formula_override_store.hpp"
#include "axiom/lhme/trace.hpp"

namespace {

class ExternalDoubleKernel final : public axiom::lhme::TechniqueKernel {
 public:
  [[nodiscard]] axiom::lhme::TechniqueIdentity identity() const override {
    return {.id = "example.double", .version = 1};
  }

  [[nodiscard]] axiom::lhme::EvaluationResult evaluate(
      const axiom::lhme::TechniqueRequest& request) const override {
    axiom::lhme::EvaluationResult result;
    result.mode = request.mode;
    result.trace = axiom::lhme::make_trace({});

    if (request.cancellation_signal != nullptr &&
        request.cancellation_signal->load()) {
      result.status = axiom::lhme::Status::Cancelled;
      return result;
    }

    if (request.identity != identity() || request.parameters != "factor=1:2" ||
        request.address.index < 0 ||
        request.mode != axiom::lhme::EvaluationMode::Exact) {
      result.status = axiom::lhme::Status::InvalidArgument;
      return result;
    }

    result.status = axiom::lhme::Status::Confirmed;
    result.integer = axiom::lhme::BigInt{request.address.index} * 2;
    return result;
  }
};

}  // namespace

int main() {
  auto engine = axiom::lhme::Engine::create({});
  const auto kernel = std::make_shared<ExternalDoubleKernel>();
  if (!engine.register_technique(kernel)) return 1;

  const auto result = engine.evaluate_technique({
      .identity = kernel->identity(),
      .parameters = "factor=1:2",
      .address = {.index = 21},
      .mode = axiom::lhme::EvaluationMode::Exact,
      .budget = axiom::lhme::Budget::unlimited(),
  });

  const auto formula = engine.register_formula(
      axiom::lhme::make_power_offset_spec(2, 100, 1));
  axiom::lhme::FormulaOverrideStore overrides{engine};
  if (overrides.set_override(formula, {.index = 4}, axiom::lhme::BigInt{9}) !=
      axiom::lhme::Status::Confirmed) {
    return 3;
  }
  const auto overridden = overrides.evaluate({
      .binding = formula,
      .address = {.index = 4},
      .mode = axiom::lhme::EvaluationMode::Exact,
      .budget = axiom::lhme::Budget::unlimited(),
  });
  const auto computed = overrides.evaluate({
      .binding = formula,
      .address = {.index = 2},
      .mode = axiom::lhme::EvaluationMode::Exact,
      .budget = axiom::lhme::Budget::unlimited(),
  });

  return result.status == axiom::lhme::Status::Confirmed &&
                 result.integer.has_value() && *result.integer == 42 &&
                 overridden.source == axiom::lhme::FormulaValueSource::Override &&
                 overridden.result.integer == axiom::lhme::BigInt{9} &&
                 computed.source == axiom::lhme::FormulaValueSource::Computed &&
                 computed.result.integer == axiom::lhme::BigInt{300}
             ? 0
             : 2;
}
