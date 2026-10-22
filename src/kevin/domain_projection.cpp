#include "axiom/lhme/domain_projection.hpp"

#include <limits>

namespace axiom::lhme {
namespace {

constexpr std::size_t kMaximumDepth = 10;

[[nodiscard]] bool is_valid_identity_character(char character) {
  const bool ascii_alphanumeric =
      (character >= 'a' && character <= 'z') ||
      (character >= 'A' && character <= 'Z') ||
      (character >= '0' && character <= '9');
  return ascii_alphanumeric || character == '.' || character == '_' ||
         character == '-';
}

[[nodiscard]] bool valid_identity_text(const std::string& id,
                                        std::uint32_t version) {
  if (id.empty() || id.size() > 128 || version == 0) return false;
  for (const auto character : id) {
    if (!is_valid_identity_character(character)) return false;
  }
  return true;
}

[[nodiscard]] std::uint64_t depth_offset(std::size_t depth) {
  std::uint64_t offset = 0;
  std::uint64_t width = 1;
  for (std::size_t level = 0; level < depth; ++level) {
    offset += width;
    width <<= 6U;
  }
  return offset;
}

}  // namespace

bool valid_grid_identity(const GridIdentity& identity) {
  return valid_identity_text(identity.id, identity.version);
}

GridAddress::GridAddress(std::vector<GridStep> steps) : steps_(std::move(steps)) {}

std::optional<GridAddress> GridAddress::from_steps(
    std::span<const GridStep> steps) {
  if (steps.size() > kMaximumDepth) return std::nullopt;
  for (const auto step : steps) {
    if (step.x > 7 || step.y > 7) return std::nullopt;
  }
  return GridAddress({steps.begin(), steps.end()});
}

std::size_t GridAddress::depth() const noexcept { return steps_.size(); }

const std::vector<GridStep>& GridAddress::steps() const noexcept {
  return steps_;
}

bool valid_grid_domain_spec(const GridDomainSpec& spec) {
  return valid_grid_identity(spec.identity) && spec.max_depth <= kMaximumDepth;
}

bool valid_projection_identity(const ProjectionIdentity& identity) {
  return valid_identity_text(identity.id, identity.version);
}

ProjectionIdentity DepthPathProjection::identity() const {
  return {.id = "axiom.depth-path", .version = 1};
}

std::optional<FormulaAddress> DepthPathProjection::project(
    const GridAddress& address) const {
  if (address.depth() > kMaximumDepth) return std::nullopt;

  std::uint64_t path_value = 0;
  for (const auto step : address.steps()) {
    path_value = (path_value << 6U) |
                 (static_cast<std::uint64_t>(step.y) << 3U) |
                 static_cast<std::uint64_t>(step.x);
  }
  const auto index = depth_offset(address.depth()) + path_value;
  if (index > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
    return std::nullopt;
  }
  return FormulaAddress{.index = static_cast<std::int64_t>(index)};
}

}  // namespace axiom::lhme
