#pragma once

#include <string>

#include "bruce/lhme/kevin_domain.hpp"
#include "bruce/lhme/technique.hpp"

namespace bruce::lhme {

// Legacy compatibility binding. New domain code uses GridDomainSpec plus a
// DomainProjection and never infers a formula value from a grid address.
struct GridSpec {
  GridIdentity identity;
  TechniqueIdentity technique;
  std::string canonical_parameters;
  std::uint8_t max_depth = 0;
  Budget default_budget = Budget::unlimited();
};

[[nodiscard]] bool valid_grid_spec(const GridSpec& spec);

}  // namespace bruce::lhme
