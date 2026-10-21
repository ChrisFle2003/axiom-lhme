#include <atomic>
#include <memory>

#include "bruce/lhme/engine.hpp"
#include "bruce/lhme/formula_override_store.hpp"
#include "bruce/lhme/trace.hpp"

namespace {

class ExternalDoubleKernel final : public bruce::lhme::TechniqueKernel {
 public:
  [[nodiscard]] bruce::lhme::TechniqueIdentity identity() const override {
    return {.id = "example.double", .version = 1};
  }

  [[nodiscard]] bruce::lhme::EvaluationResult evaluate(
      const bruce::lhme::TechniqueRequest& request) const override {
    bruce::lhme::EvaluationResult result;
    result.mode = request.mode;
    result.trace = bruce::lhme::make_trace({});

    if (request.cancellation_signal != nullptr &&
        request.cancellation_signal->load()) {
      result.status = bruce::lhme::Status::Cancelled;
      return result;
    }

    if (request.identity != identity() || request.parameters != "factor=1:2" ||
        request.address.index < 0 ||
        request.mode != bruce::lhme::EvaluationMode::Exact) {
      result.status = bruce::lhme::Status::InvalidArgument;
      return result;
    }

    result.status = bruce::lhme::Status::Confirmed;
    result.integer = bruce::lhme::BigInt{request.address.index} * 2;
    return result;
  }
};

}  // namespace

int main() {
  auto engine = bruce::lhme::Engine::create({});
  const auto kernel = std::make_shared<ExternalDoubleKernel>();
  if (!engine.register_technique(kernel)) return 1;

  const auto result = engine.evaluate_technique({
      .identity = kernel->identity(),
      .parameters = "factor=1:2",
      .address = {.index = 21},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited(),
  });

  const auto formula = engine.register_formula(
      bruce::lhme::make_power_offset_spec(2, 100, 1));
  bruce::lhme::FormulaOverrideStore overrides{engine};
  if (overrides.set_override(formula, {.index = 4}, bruce::lhme::BigInt{9}) !=
      bruce::lhme::Status::Confirmed) {
    return 3;
  }
  const auto overridden = overrides.evaluate({
      .binding = formula,
      .address = {.index = 4},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited(),
  });
  const auto computed = overrides.evaluate({
      .binding = formula,
      .address = {.index = 2},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited(),
  });

  return result.status == bruce::lhme::Status::Confirmed &&
                 result.integer.has_value() && *result.integer == 42 &&
                 overridden.source == bruce::lhme::FormulaValueSource::Override &&
                 overridden.result.integer == bruce::lhme::BigInt{9} &&
                 computed.source == bruce::lhme::FormulaValueSource::Computed &&
                 computed.result.integer == bruce::lhme::BigInt{300}
             ? 0
             : 2;
}
