#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "bruce/lhme/kevin_domain.hpp"

namespace bruce::lhme {

struct ProjectionIdentity {
  std::string id;
  std::uint32_t version = 0;

  auto operator==(const ProjectionIdentity&) const -> bool = default;
};

[[nodiscard]] bool valid_projection_identity(const ProjectionIdentity& identity);

class DomainProjection {
 public:
  virtual ~DomainProjection() = default;

  [[nodiscard]] virtual ProjectionIdentity identity() const = 0;
  [[nodiscard]] virtual std::optional<FormulaAddress> project(
      const GridAddress& address) const = 0;
};

class DepthPathProjection final : public DomainProjection {
 public:
  [[nodiscard]] ProjectionIdentity identity() const override;
  [[nodiscard]] std::optional<FormulaAddress> project(
      const GridAddress& address) const override;
};

}  // namespace bruce::lhme
