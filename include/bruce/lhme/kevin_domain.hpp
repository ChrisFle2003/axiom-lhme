#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "bruce/lhme/types.hpp"

namespace bruce::lhme {

struct GridIdentity {
  std::string id;
  std::uint32_t version = 0;

  auto operator==(const GridIdentity&) const -> bool = default;
};

[[nodiscard]] bool valid_grid_identity(const GridIdentity& identity);

struct GridStep {
  std::uint8_t x = 0;
  std::uint8_t y = 0;

  auto operator==(const GridStep&) const -> bool = default;
};

class GridAddress {
 public:
  [[nodiscard]] static std::optional<GridAddress> from_steps(
      std::span<const GridStep> steps);

  [[nodiscard]] std::size_t depth() const noexcept;
  [[nodiscard]] const std::vector<GridStep>& steps() const noexcept;

  [[nodiscard]] bool operator==(const GridAddress& other) const noexcept {
    return steps_ == other.steps_;
  }

 private:
  explicit GridAddress(std::vector<GridStep> steps);

  std::vector<GridStep> steps_;
};

struct GridDomainSpec {
  GridIdentity identity;
  std::uint8_t max_depth = 0;
  Budget default_budget = Budget::unlimited();
};

[[nodiscard]] bool valid_grid_domain_spec(const GridDomainSpec& spec);

}  // namespace bruce::lhme
