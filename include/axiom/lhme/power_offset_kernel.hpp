#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "axiom/lhme/technique.hpp"

namespace axiom::lhme {

struct PowerOffsetParameters {
  BigInt base;
  BigInt scale;
};

[[nodiscard]] inline std::string encode_power_offset_parameters(
    const BigInt& base, const BigInt& scale) {
  if (base <= 0 || scale < 0) {
    throw std::invalid_argument("power-offset parameters require base > 0 and scale >= 0");
  }
  const auto base_text = base.convert_to<std::string>();
  const auto scale_text = scale.convert_to<std::string>();
  return "base=" + std::to_string(base_text.size()) + ":" + base_text +
         ";scale=" + std::to_string(scale_text.size()) + ":" + scale_text;
}

[[nodiscard]] std::optional<PowerOffsetParameters> parse_power_offset_parameters(
    std::string_view parameters);

class PowerOffsetKernel final : public TechniqueKernel {
 public:
  [[nodiscard]] TechniqueIdentity identity() const override;
  [[nodiscard]] EvaluationResult evaluate(
      const TechniqueRequest& request) const override;
};

}  // namespace axiom::lhme
