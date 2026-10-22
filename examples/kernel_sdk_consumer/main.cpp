#include <memory>

#include "axiom/lhme/engine.hpp"
#include "axiom/lhme/kernel_sdk.hpp"

namespace {

class AffineKernel final : public axiom::lhme::TechniqueKernel {
 public:
  [[nodiscard]] axiom::lhme::TechniqueIdentity identity() const override {
    return {.id = "example.affine", .version = 1};
  }

  [[nodiscard]] axiom::lhme::EvaluationResult evaluate(
      const axiom::lhme::TechniqueRequest& request) const override {
    if (axiom::lhme::cancellation_requested(request)) {
      return axiom::lhme::cancelled_technique_result(request.mode,
                                                      "kernel cancellation requested");
    }
    if (request.identity != identity() ||
        request.parameters != "factor=1:2;offset=1:3" ||
        request.address.index < 0 ||
        request.mode != axiom::lhme::EvaluationMode::Exact) {
      return axiom::lhme::invalid_technique_request_result(
          request.mode, "expected exact affine request");
    }

    return axiom::lhme::confirmed_exact_result(
        axiom::lhme::BigInt{request.address.index} * 2 + 3,
        axiom::lhme::make_trace({}));
  }
};

}  // namespace

int main() {
  auto engine = axiom::lhme::Engine::create({});
  const auto kernel = std::make_shared<AffineKernel>();
  if (!engine.register_technique(kernel)) return 1;

  const auto result = engine.evaluate_technique({
      .identity = kernel->identity(),
      .parameters = "factor=1:2;offset=1:3",
      .address = {.index = 21},
      .mode = axiom::lhme::EvaluationMode::Exact,
      .budget = axiom::lhme::Budget::unlimited(),
  });

  return result.status == axiom::lhme::Status::Confirmed &&
                 result.integer.has_value() && *result.integer == 45
             ? 0
             : 2;
}
