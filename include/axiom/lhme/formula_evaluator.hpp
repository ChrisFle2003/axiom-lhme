#pragma once

#include <optional>

#include "axiom/lhme/types.hpp"

namespace axiom::lhme {

class FormulaEvaluator {
 public:
  virtual ~FormulaEvaluator() = default;

  [[nodiscard]] virtual EvaluationResult evaluate(
      FormulaAddress address, EvaluationMode mode, Budget budget,
      std::optional<BigInt> modulus = std::nullopt) = 0;
};

}  // namespace axiom::lhme
