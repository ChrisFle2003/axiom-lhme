#include "test.hpp"

#include "bruce/lhme/gguf_storage_plan.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace {

using bruce::lhme::GgufInventory;
using bruce::lhme::GgufLoadingPolicy;
using bruce::lhme::GgufPlacement;
using bruce::lhme::GgufPlanConfig;
using bruce::lhme::GgufTensor;
using bruce::lhme::Status;
using bruce::lhme::make_gguf_storage_plan;

GgufInventory three_tensor_inventory() {
  return {.version = 3,
          .alignment = 32,
          .file_byte_count = 128,
          .input_fingerprint = "fixture",
          .tensors = {
              {.name = "late",
               .shape = {4},
               .ggml_type = 0,
               .logical_element_count = 4,
               .relative_data_offset = 32,
               .absolute_file_offset = 64,
               .stored_byte_count = 16,
               .decoded_ram_bytes = 64},
              {.name = "first",
               .shape = {4},
               .ggml_type = 0,
               .logical_element_count = 4,
               .relative_data_offset = 0,
               .absolute_file_offset = 32,
               .stored_byte_count = 16,
               .decoded_ram_bytes = 64},
              {.name = "profiled",
               .shape = {8},
               .ggml_type = 0,
               .logical_element_count = 8,
               .relative_data_offset = 64,
               .absolute_file_offset = 96,
               .stored_byte_count = 32,
               .decoded_ram_bytes = 128},
          }};
}

}  // namespace

TEST(gguf_storage_plan_rejects_an_all_resident_plan_that_exceeds_its_budget) {
  const auto plan = make_gguf_storage_plan(
      three_tensor_inventory(), {.memory_budget_bytes = 255,
                                  .chunk_size_bytes = 32,
                                  .policy = GgufLoadingPolicy::AllResident});

  CHECK(plan.status == Status::BudgetExceeded);
  CHECK(plan.resident_ram_bytes == 0U);
  CHECK(plan.items.size() == 3U);
  for (const auto& item : plan.items) {
    CHECK(item.placement == GgufPlacement::Rejected);
  }
}

TEST(gguf_storage_plan_prioritizes_profile_then_file_offset_under_a_budget) {
  const auto plan = make_gguf_storage_plan(
      three_tensor_inventory(), {.memory_budget_bytes = 128,
                                  .chunk_size_bytes = 32,
                                  .policy = GgufLoadingPolicy::Budgeted,
                                  .ordered_tensor_use_profile = {"profiled"}});

  CHECK(plan.status == Status::Confirmed);
  CHECK(plan.resident_ram_bytes == 128U);
  CHECK(plan.items.size() == 3U);
  CHECK(plan.items[0].tensor_name == "profiled");
  CHECK(plan.items[0].placement == GgufPlacement::Resident);
  CHECK(plan.items[1].tensor_name == "first");
  CHECK(plan.items[1].placement == GgufPlacement::OnDemand);
  CHECK(plan.items[2].tensor_name == "late");
  CHECK(plan.items[2].placement == GgufPlacement::OnDemand);
}

TEST(gguf_storage_plan_defers_every_chunk_for_metadata_only) {
  const auto plan = make_gguf_storage_plan(
      three_tensor_inventory(), {.memory_budget_bytes = 0,
                                  .chunk_size_bytes = 32,
                                  .policy = GgufLoadingPolicy::MetadataOnly});

  CHECK(plan.status == Status::Confirmed);
  CHECK(plan.resident_ram_bytes == 0U);
  for (const auto& item : plan.items) {
    CHECK(item.placement == GgufPlacement::Deferred);
  }
}

TEST(gguf_storage_plan_chunks_tensor_spans_with_proportional_ram_estimates) {
  GgufInventory inventory = three_tensor_inventory();
  inventory.tensors = {inventory.tensors[1]};
  const auto plan = make_gguf_storage_plan(
      inventory, {.memory_budget_bytes = 64,
                  .chunk_size_bytes = 8,
                  .policy = GgufLoadingPolicy::Budgeted});

  CHECK(plan.status == Status::Confirmed);
  CHECK(plan.items.size() == 2U);
  CHECK(plan.items[0].stored_byte_count == 8U);
  CHECK(plan.items[1].stored_byte_count == 8U);
  CHECK(plan.items[0].decoded_ram_bytes == 32U);
  CHECK(plan.items[1].decoded_ram_bytes == 32U);
  CHECK(plan.items[0].placement == GgufPlacement::Resident);
  CHECK(plan.items[1].placement == GgufPlacement::Resident);
}

TEST(gguf_storage_plan_is_deterministic_for_equal_inputs) {
  const GgufPlanConfig config{.memory_budget_bytes = 128,
                              .chunk_size_bytes = 32,
                              .policy = GgufLoadingPolicy::Budgeted,
                              .ordered_tensor_use_profile = {"profiled"}};
  const auto first = make_gguf_storage_plan(three_tensor_inventory(), config);
  const auto second = make_gguf_storage_plan(three_tensor_inventory(), config);

  CHECK(first.status == Status::Confirmed);
  CHECK(first.trace->id == second.trace->id);
  CHECK(first.items.size() == second.items.size());
  for (std::size_t index = 0; index < first.items.size(); ++index) {
    CHECK(first.items[index].tensor_name == second.items[index].tensor_name);
    CHECK(first.items[index].file_offset == second.items[index].file_offset);
    CHECK(first.items[index].placement == second.items[index].placement);
  }
}

TEST(gguf_storage_plan_rejects_duplicate_or_unknown_profile_names) {
  const auto duplicate = make_gguf_storage_plan(
      three_tensor_inventory(), {.memory_budget_bytes = 128,
                                  .chunk_size_bytes = 32,
                                  .policy = GgufLoadingPolicy::Budgeted,
                                  .ordered_tensor_use_profile = {"first", "first"}});
  const auto unknown = make_gguf_storage_plan(
      three_tensor_inventory(), {.memory_budget_bytes = 128,
                                  .chunk_size_bytes = 32,
                                  .policy = GgufLoadingPolicy::Budgeted,
                                  .ordered_tensor_use_profile = {"missing"}});

  CHECK(duplicate.status == Status::InvalidArgument);
  CHECK(unknown.status == Status::InvalidArgument);
}
