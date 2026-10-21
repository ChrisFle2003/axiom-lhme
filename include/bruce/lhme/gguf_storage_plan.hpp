#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bruce/lhme/gguf_inspector.hpp"
#include "bruce/lhme/trace.hpp"
#include "bruce/lhme/types.hpp"

namespace bruce::lhme {

enum class GgufLoadingPolicy {
  AllResident,
  Budgeted,
  MetadataOnly,
};

enum class GgufPlacement {
  Resident,
  OnDemand,
  Deferred,
  Rejected,
};

struct GgufPlanConfig {
  std::uint64_t memory_budget_bytes = 0;
  std::uint64_t chunk_size_bytes = 0;
  GgufLoadingPolicy policy = GgufLoadingPolicy::Budgeted;
  std::vector<std::string> ordered_tensor_use_profile;
};

struct GgufPlanItem {
  std::string tensor_name;
  std::uint64_t chunk_index = 0;
  std::uint64_t file_offset = 0;
  std::uint64_t stored_byte_count = 0;
  std::uint64_t decoded_ram_bytes = 0;
  GgufPlacement placement = GgufPlacement::Rejected;
  std::string reason;
};

struct GgufStoragePlan {
  Status status = Status::InternalError;
  std::uint64_t memory_budget_bytes = 0;
  std::uint64_t resident_ram_bytes = 0;
  std::string diagnostic;
  std::vector<GgufPlanItem> items;
  std::shared_ptr<const TraceRecord> trace;
};

[[nodiscard]] GgufStoragePlan make_gguf_storage_plan(
    const GgufInventory& inventory, const GgufPlanConfig& config);

}  // namespace bruce::lhme
