#pragma once

#include <optional>

#include "bruce/lhme/types.hpp"

namespace bruce::lhme {

class FormulaEvaluator {
 public:
  virtual ~FormulaEvaluator() = default;

  [[nodiscard]] virtual EvaluationResult evaluate(
      FormulaAddress address, EvaluationMode mode, Budget budget,
      std::optional<BigInt> modulus = std::nullopt) = 0;
};

}  // namespace bruce::lhme
