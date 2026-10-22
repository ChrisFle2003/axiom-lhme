#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "axiom/lhme/types.hpp"

namespace axiom::lhme {

enum class FormulaOperation {
  PowerOffset,
};

struct FormulaSpec {
  FormulaOperation operation = FormulaOperation::PowerOffset;
  BigInt base = 0;
  BigInt scale = 0;
  bool nonnegative_index = true;
  std::uint32_t version = 0;
  std::array<std::byte, 32> specification_hash{};
};

[[nodiscard]] FormulaSpec make_power_offset_spec(BigInt base, BigInt scale,
                                                  std::uint32_t version);
[[nodiscard]] std::string canonical_formula_identity(const FormulaSpec& spec);

[[nodiscard]] EvaluationResult evaluate_power_offset(
    const FormulaSpec& spec, const FormulaAddress& address,
    EvaluationMode mode, const Budget& budget);

struct EvaluationRequest {
  FormulaSpec spec;
  FormulaAddress address;
  EvaluationMode mode = EvaluationMode::Symbolic;
  Budget budget = Budget::unlimited();
  std::optional<BigInt> modulus;
  bool cancellation_requested = false;
  std::shared_ptr<const std::atomic_bool> cancellation_signal;
};

[[nodiscard]] EvaluationResult evaluate(const EvaluationRequest& request);

}  // namespace axiom::lhme
