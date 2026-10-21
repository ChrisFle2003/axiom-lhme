#include <memory>

#include "bruce/lhme/engine.hpp"
#include "bruce/lhme/kernel_sdk.hpp"

namespace {

class AffineKernel final : public bruce::lhme::TechniqueKernel {
 public:
  [[nodiscard]] bruce::lhme::TechniqueIdentity identity() const override {
    return {.id = "example.affine", .version = 1};
  }

  [[nodiscard]] bruce::lhme::EvaluationResult evaluate(
      const bruce::lhme::TechniqueRequest& request) const override {
    if (bruce::lhme::cancellation_requested(request)) {
      return bruce::lhme::cancelled_technique_result(request.mode,
                                                      "kernel cancellation requested");
    }
    if (request.identity != identity() ||
        request.parameters != "factor=1:2;offset=1:3" ||
        request.address.index < 0 ||
        request.mode != bruce::lhme::EvaluationMode::Exact) {
      return bruce::lhme::invalid_technique_request_result(
          request.mode, "expected exact affine request");
    }

    return bruce::lhme::confirmed_exact_result(
        bruce::lhme::BigInt{request.address.index} * 2 + 3,
        bruce::lhme::make_trace({}));
  }
};

}  // namespace

int main() {
  auto engine = bruce::lhme::Engine::create({});
  const auto kernel = std::make_shared<AffineKernel>();
  if (!engine.register_technique(kernel)) return 1;

  const auto result = engine.evaluate_technique({
      .identity = kernel->identity(),
      .parameters = "factor=1:2;offset=1:3",
      .address = {.index = 21},
      .mode = bruce::lhme::EvaluationMode::Exact,
      .budget = bruce::lhme::Budget::unlimited(),
  });

  return result.status == bruce::lhme::Status::Confirmed &&
                 result.integer.has_value() && *result.integer == 45
             ? 0
             : 2;
}
