#include "test.hpp"

#include "axiom/lhme/domain_projection.hpp"
#include "axiom/lhme/kevin_grid.hpp"

#include <vector>

namespace {

using axiom::lhme::GridAddress;
using axiom::lhme::GridIdentity;
using axiom::lhme::GridStep;
using axiom::lhme::DepthPathProjection;
using axiom::lhme::valid_grid_identity;

}  // namespace

TEST(grid_identity_requires_ascii_name_and_positive_version) {
  CHECK(valid_grid_identity({.id = "axiom.grid", .version = 1}));
  CHECK(!valid_grid_identity({.id = "", .version = 1}));
  CHECK(!valid_grid_identity({.id = "axiom.grid", .version = 0}));
  CHECK(!valid_grid_identity({.id = "axiom/grid", .version = 1}));
}

TEST(depth_path_projection_encodes_path_depth_without_zero_step_collisions) {
  const std::vector<GridStep> empty_path;
  const std::vector<GridStep> one_zero_step{{.x = 0, .y = 0}};
  const std::vector<GridStep> two_zero_steps{{.x = 0, .y = 0}, {.x = 0, .y = 0}};
  const std::vector<GridStep> one_maximum_step{{.x = 7, .y = 7}};

  const auto empty = GridAddress::from_steps(empty_path);
  const auto one_zero = GridAddress::from_steps(one_zero_step);
  const auto two_zero = GridAddress::from_steps(two_zero_steps);
  const auto one_maximum = GridAddress::from_steps(one_maximum_step);

  CHECK(empty.has_value());
  CHECK(one_zero.has_value());
  CHECK(two_zero.has_value());
  CHECK(one_maximum.has_value());
  DepthPathProjection projection;
  CHECK(projection.project(*empty)->index == 0);
  CHECK(projection.project(*one_zero)->index == 1);
  CHECK(projection.project(*two_zero)->index == 65);
  CHECK(projection.project(*one_maximum)->index == 64);
}

TEST(grid_address_rejects_invalid_coordinates_and_excessive_depth) {
  const std::vector<GridStep> invalid_coordinate{{.x = 8, .y = 0}};
  const std::vector<GridStep> too_deep(11, GridStep{.x = 7, .y = 7});

  CHECK(!GridAddress::from_steps(invalid_coordinate).has_value());
  CHECK(!GridAddress::from_steps(too_deep).has_value());
}

TEST(depth_path_projection_accepts_the_largest_v14_path_as_a_nonnegative_index) {
  const std::vector<GridStep> deepest_path(10, GridStep{.x = 7, .y = 7});
  const auto address = GridAddress::from_steps(deepest_path);

  CHECK(address.has_value());
  CHECK(address->depth() == 10);
  DepthPathProjection projection;
  CHECK(projection.project(*address)->index > 0);
}
