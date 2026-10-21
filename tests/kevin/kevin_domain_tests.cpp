#include "test.hpp"

#include "bruce/lhme/domain_projection.hpp"
#include "bruce/lhme/kevin_domain.hpp"

#include <vector>

namespace {

using bruce::lhme::DepthPathProjection;
using bruce::lhme::GridAddress;
using bruce::lhme::GridDomainSpec;
using bruce::lhme::GridIdentity;
using bruce::lhme::GridStep;
using bruce::lhme::valid_grid_domain_spec;

}  // namespace

TEST(depth_path_projection_maps_a_path_only_when_explicitly_requested) {
  const std::vector<GridStep> steps{{.x = 3, .y = 5}};
  const auto address = GridAddress::from_steps(steps);

  CHECK(address.has_value());
  DepthPathProjection projection;
  const auto formula_address = projection.project(*address);

  CHECK(formula_address.has_value());
  CHECK(formula_address->index == 1 + 43);
}

TEST(two_domains_can_share_a_grid_coordinate_without_a_value_binding) {
  const std::vector<GridStep> steps{{.x = 3, .y = 5}};
  const auto address = GridAddress::from_steps(steps);
  const GridDomainSpec first{{.id = "bruce.domain.first", .version = 1}, 4};
  const GridDomainSpec second{{.id = "bruce.domain.second", .version = 1}, 4};

  CHECK(address.has_value());
  CHECK(valid_grid_domain_spec(first));
  CHECK(valid_grid_domain_spec(second));
  CHECK(first.identity != second.identity);
  CHECK(address->steps() == steps);
}
