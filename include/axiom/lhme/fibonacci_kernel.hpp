#pragma once

#include "axiom/lhme/technique.hpp"

namespace axiom::lhme {

inline constexpr char kFibonacciFastDoublingParameters[] =
    "algorithm=8:fast_dbl";

class FibonacciKernel final : public TechniqueKernel {
 public:
  [[nodiscard]] TechniqueIdentity identity() const override;
  [[nodiscard]] EvaluationResult evaluate(
      const TechniqueRequest& request) const override;
};

}  // namespace axiom::lhme
