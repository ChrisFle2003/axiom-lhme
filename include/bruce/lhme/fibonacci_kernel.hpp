#pragma once

#include "bruce/lhme/technique.hpp"

namespace bruce::lhme {

inline constexpr char kFibonacciFastDoublingParameters[] =
    "algorithm=8:fast_dbl";

class FibonacciKernel final : public TechniqueKernel {
 public:
  [[nodiscard]] TechniqueIdentity identity() const override;
  [[nodiscard]] EvaluationResult evaluate(
      const TechniqueRequest& request) const override;
};

}  // namespace bruce::lhme
