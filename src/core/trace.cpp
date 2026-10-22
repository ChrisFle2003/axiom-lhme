#include "axiom/lhme/trace.hpp"

#include <cstdint>
#include <utility>

namespace axiom::lhme {

bool TraceRecord::contains(GateScope scope) const {
  for (const auto& event : events) {
    if (event.scope == scope) {
      return true;
    }
  }
  return false;
}

std::shared_ptr<const TraceRecord> make_trace(std::vector<GateEvent> events) {
  std::uint64_t hash = 14695981039346656037ULL;
  for (const auto& event : events) {
    hash ^= static_cast<std::uint64_t>(event.scope);
    hash *= 1099511628211ULL;
    hash ^= static_cast<std::uint64_t>(event.status);
    hash *= 1099511628211ULL;
    for (const auto character : event.reason) {
      hash ^= static_cast<unsigned char>(character);
      hash *= 1099511628211ULL;
    }
  }
  return std::make_shared<const TraceRecord>(
      TraceRecord{.id = std::to_string(hash), .events = std::move(events)});
}

}  // namespace axiom::lhme
