#include "bruce/lhme/gguf_storage_plan.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace bruce::lhme {
namespace {

[[nodiscard]] bool checked_add(std::uint64_t left, std::uint64_t right,
                               std::uint64_t& result) {
  if (left > std::numeric_limits<std::uint64_t>::max() - right) return false;
  result = left + right;
  return true;
}

[[nodiscard]] std::uint64_t scaled_prefix(std::uint64_t total,
                                           std::uint64_t position,
                                           std::uint64_t extent) {
  const BigInt scaled = BigInt{total} * position / extent;
  return scaled.convert_to<std::uint64_t>();
}

[[nodiscard]] GgufStoragePlan invalid_plan(Status status, std::string diagnostic) {
  return {.status = status,
          .memory_budget_bytes = 0,
          .resident_ram_bytes = 0,
          .diagnostic = std::move(diagnostic),
          .items = {},
          .trace = make_trace({{.scope = GateScope::Run,
                                .status = status,
                                .reason = "invalid gguf storage plan"}})};
}

[[nodiscard]] bool item_order(const GgufPlanItem& left,
                              const GgufPlanItem& right) {
  if (left.file_offset != right.file_offset) return left.file_offset < right.file_offset;
  if (left.tensor_name != right.tensor_name) return left.tensor_name < right.tensor_name;
  return left.chunk_index < right.chunk_index;
}

}  // namespace

GgufStoragePlan make_gguf_storage_plan(const GgufInventory& inventory,
                                       const GgufPlanConfig& config) {
  if (config.chunk_size_bytes == 0) {
    return invalid_plan(Status::InvalidArgument, "chunk size must be nonzero");
  }

  std::set<std::string> inventory_names;
  for (const auto& tensor : inventory.tensors) {
    if (tensor.name.empty() || !tensor.decoded_ram_bytes.has_value() ||
        tensor.stored_byte_count == 0 || !inventory_names.insert(tensor.name).second) {
      return invalid_plan(Status::InvalidArgument, "inventory is not plannable");
    }
  }

  std::map<std::string, std::size_t> profile_priority;
  for (std::size_t index = 0; index < config.ordered_tensor_use_profile.size(); ++index) {
    const auto& name = config.ordered_tensor_use_profile[index];
    if (!inventory_names.contains(name) || !profile_priority.emplace(name, index).second) {
      return invalid_plan(Status::InvalidArgument, "use profile contains duplicate or unknown tensor");
    }
  }

  std::vector<GgufTensor> tensors = inventory.tensors;
  std::sort(tensors.begin(), tensors.end(), [](const GgufTensor& left,
                                               const GgufTensor& right) {
    return left.absolute_file_offset == right.absolute_file_offset
               ? left.name < right.name
               : left.absolute_file_offset < right.absolute_file_offset;
  });

  std::vector<GgufPlanItem> items;
  for (const auto& tensor : tensors) {
    std::uint64_t chunk_offset = 0;
    std::uint64_t chunk_index = 0;
    while (chunk_offset < tensor.stored_byte_count) {
      const auto chunk_bytes = std::min(config.chunk_size_bytes,
                                        tensor.stored_byte_count - chunk_offset);
      std::uint64_t chunk_end = 0;
      std::uint64_t file_offset = 0;
      if (!checked_add(chunk_offset, chunk_bytes, chunk_end) ||
          !checked_add(tensor.absolute_file_offset, chunk_offset, file_offset)) {
        return invalid_plan(Status::InvalidArgument, "chunk range overflows uint64");
      }
      const auto decoded_start = scaled_prefix(*tensor.decoded_ram_bytes, chunk_offset,
                                               tensor.stored_byte_count);
      const auto decoded_end = scaled_prefix(*tensor.decoded_ram_bytes, chunk_end,
                                             tensor.stored_byte_count);
      items.push_back({.tensor_name = tensor.name,
                       .chunk_index = chunk_index,
                       .file_offset = file_offset,
                       .stored_byte_count = chunk_bytes,
                       .decoded_ram_bytes = decoded_end - decoded_start,
                       .placement = GgufPlacement::Rejected,
                       .reason = {}});
      chunk_offset = chunk_end;
      ++chunk_index;
    }
  }
  std::sort(items.begin(), items.end(), item_order);

  std::vector<GgufPlanItem*> ordered_items;
  ordered_items.reserve(items.size());
  for (auto& item : items) ordered_items.push_back(&item);
  if (config.policy == GgufLoadingPolicy::Budgeted) {
    std::stable_sort(ordered_items.begin(), ordered_items.end(),
                     [&profile_priority](const GgufPlanItem* left,
                                         const GgufPlanItem* right) {
                       const auto left_profile = profile_priority.find(left->tensor_name);
                       const auto right_profile = profile_priority.find(right->tensor_name);
                       if (left_profile != profile_priority.end() &&
                           right_profile != profile_priority.end()) {
                         if (left_profile->second != right_profile->second) {
                           return left_profile->second < right_profile->second;
                         }
                       } else if (left_profile != profile_priority.end()) {
                         return true;
                       } else if (right_profile != profile_priority.end()) {
                         return false;
                       }
                       return item_order(*left, *right);
                     });
  }

  GgufStoragePlan plan;
  plan.status = Status::Confirmed;
  plan.memory_budget_bytes = config.memory_budget_bytes;
  plan.items.reserve(items.size());

  if (config.policy == GgufLoadingPolicy::MetadataOnly) {
    for (auto& item : items) {
      item.placement = GgufPlacement::Deferred;
      item.reason = "metadata-only policy";
    }
  } else if (config.policy == GgufLoadingPolicy::AllResident) {
    std::uint64_t total_ram = 0;
    for (const auto& item : items) {
      if (!checked_add(total_ram, item.decoded_ram_bytes, total_ram)) {
        return invalid_plan(Status::InvalidArgument, "planned RAM total overflows uint64");
      }
    }
    if (total_ram > config.memory_budget_bytes) {
      plan.status = Status::BudgetExceeded;
      plan.diagnostic = "all-resident plan exceeds memory budget";
      for (auto& item : items) {
        item.placement = GgufPlacement::Rejected;
        item.reason = "all-resident budget exceeded";
      }
    } else {
      plan.resident_ram_bytes = total_ram;
      for (auto& item : items) {
        item.placement = GgufPlacement::Resident;
        item.reason = "all-resident policy";
      }
    }
  } else {
    for (auto* item : ordered_items) {
      std::uint64_t next_resident_bytes = 0;
      if (checked_add(plan.resident_ram_bytes, item->decoded_ram_bytes,
                      next_resident_bytes) &&
          next_resident_bytes <= config.memory_budget_bytes) {
        plan.resident_ram_bytes = next_resident_bytes;
        item->placement = GgufPlacement::Resident;
        item->reason = profile_priority.contains(item->tensor_name)
                           ? "profile-priority resident"
                           : "offset-order resident";
      } else {
        item->placement = GgufPlacement::OnDemand;
        item->reason = "memory budget reserved for on-demand access";
      }
    }
    std::vector<GgufPlanItem> prioritized_items;
    prioritized_items.reserve(ordered_items.size());
    for (const auto* item : ordered_items) prioritized_items.push_back(*item);
    items = std::move(prioritized_items);
  }

  std::vector<GateEvent> events;
  events.reserve(items.size() + 1U);
  events.push_back({.scope = GateScope::Run,
                    .status = plan.status,
                    .reason = "gguf storage plan"});
  for (const auto& item : items) {
    events.push_back({.scope = GateScope::Tile,
                      .status = plan.status,
                      .reason = item.reason});
  }
  plan.items = std::move(items);
  plan.trace = make_trace(std::move(events));
  return plan;
}

}  // namespace bruce::lhme
