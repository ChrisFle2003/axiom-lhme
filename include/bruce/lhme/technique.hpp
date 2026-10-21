#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "bruce/lhme/formula.hpp"

namespace bruce::lhme {

struct TechniqueIdentity {
  std::string id;
  std::uint32_t version = 0;

  auto operator==(const TechniqueIdentity&) const -> bool = default;
};

[[nodiscard]] inline bool valid_technique_identity(const TechniqueIdentity& identity) {
  if (identity.id.empty() || identity.id.size() > 128 || identity.version == 0) {
    return false;
  }
  for (const auto character : identity.id) {
    const bool ascii_alphanumeric =
        (character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z') ||
        (character >= '0' && character <= '9');
    if (!ascii_alphanumeric && character != '.' && character != '_' && character != '-') {
      return false;
    }
  }
  return true;
}

struct TechniqueRequest {
  TechniqueIdentity identity;
  std::string parameters;
  FormulaAddress address;
  EvaluationMode mode = EvaluationMode::Symbolic;
  Budget budget = Budget::unlimited();
  std::optional<BigInt> modulus;
  std::shared_ptr<const std::atomic_bool> cancellation_signal;
};

class TechniqueKernel {
 public:
  virtual ~TechniqueKernel() = default;

  [[nodiscard]] virtual TechniqueIdentity identity() const = 0;
  [[nodiscard]] virtual EvaluationResult evaluate(
      const TechniqueRequest& request) const = 0;
};

}  // namespace bruce::lhme
