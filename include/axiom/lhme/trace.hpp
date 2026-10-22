#pragma once

#include <memory>
#include <string>
#include <vector>

#include "axiom/lhme/types.hpp"

namespace axiom::lhme {

enum class GateScope {
  Branch,
  Tile,
  Run,
  Cache,
  Publish,
  Projection,
};

struct GateEvent {
  GateScope scope = GateScope::Branch;
  Status status = Status::InternalError;
  std::string reason;
};

struct TraceRecord {
  std::string id;
  std::vector<GateEvent> events;

  [[nodiscard]] bool contains(GateScope scope) const;
};

[[nodiscard]] std::shared_ptr<const TraceRecord> make_trace(
    std::vector<GateEvent> events);

}  // namespace axiom::lhme
