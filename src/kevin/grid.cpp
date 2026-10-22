#include "axiom/lhme/kevin_grid.hpp"

namespace axiom::lhme {
namespace {

constexpr std::size_t kMaximumDepth = 10;

}  // namespace

bool valid_grid_spec(const GridSpec& spec) {
  return valid_grid_identity(spec.identity) && valid_technique_identity(spec.technique) &&
         !spec.canonical_parameters.empty() &&
         spec.canonical_parameters.size() <= 4096 && spec.max_depth <= kMaximumDepth;
}

}  // namespace axiom::lhme
