#include "test.hpp"

#include "gguf_fixture.hpp"

#include "bruce/lhme/gguf_pattern_scan.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace {

using bruce::lhme::GgufInspectionLimits;
using bruce::lhme::GgufPatternKind;
using bruce::lhme::GgufPatternScanConfig;
using bruce::lhme::GgufPlanItem;
using bruce::lhme::Status;
using bruce::lhme::inspect_gguf_file;
using bruce::lhme::scan_gguf_pattern_candidates;

std::vector<std::byte> f32_file(std::initializer_list<float> values) {
  auto bytes = test::gguf::tensor_file({values.size()}, test::gguf::kTypeF32,
                                       values.size() * sizeof(float));
  auto output = bytes.end() - static_cast<std::ptrdiff_t>(values.size() * sizeof(float));
  for (const auto value : values) {
    const auto bits = std::bit_cast<std::uint32_t>(value);
    for (std::size_t index = 0; index < sizeof(bits); ++index) {
      *output++ = static_cast<std::byte>((bits >> (index * 8U)) & 0xffU);
    }
  }
  return bytes;
}

GgufPlanItem selected_chunk(const bruce::lhme::GgufInventory& inventory) {
  const auto& tensor = inventory.tensors.front();
  return {.tensor_name = tensor.name,
          .chunk_index = 0,
          .file_offset = tensor.absolute_file_offset,
          .stored_byte_count = tensor.stored_byte_count,
          .decoded_ram_bytes = *tensor.decoded_ram_bytes};
}

bool has_kind(const bruce::lhme::GgufPatternScanResult& result,
              GgufPatternKind kind) {
  return std::any_of(result.candidates.begin(), result.candidates.end(),
                     [kind](const auto& candidate) { return candidate.kind == kind; });
}

}  // namespace

TEST(gguf_pattern_scan_reports_constant_f32_bytes_without_verifying_a_recipe) {
  const auto path = test::gguf::write_file("constant", f32_file({5.0F, 5.0F, 5.0F, 5.0F}));
  const auto inspection = inspect_gguf_file(path, GgufInspectionLimits{});
  const auto chunk = selected_chunk(*inspection.inventory);

  const auto result = scan_gguf_pattern_candidates(
      path, *inspection.inventory, {&chunk, 1},
      {.max_payload_bytes = 16, .max_candidates = 8});

  CHECK(result.status == Status::Confirmed);
  CHECK(has_kind(result, GgufPatternKind::ConstantBytes));
  for (const auto& candidate : result.candidates) {
    CHECK(candidate.tensor_name == "weight");
    CHECK(!candidate.exactly_verified);
  }
}

TEST(gguf_pattern_scan_reports_repeated_bytes_and_exact_f32_progression_candidates) {
  const auto repeated_path = test::gguf::write_file("repeated", f32_file({1.0F, 2.0F, 1.0F, 2.0F}));
  const auto repeated_inspection = inspect_gguf_file(repeated_path, GgufInspectionLimits{});
  const auto repeated_chunk = selected_chunk(*repeated_inspection.inventory);
  const auto repeated = scan_gguf_pattern_candidates(
      repeated_path, *repeated_inspection.inventory, {&repeated_chunk, 1},
      {.max_payload_bytes = 16, .max_candidates = 8});

  const auto progression_path = test::gguf::write_file("progression", f32_file({1.0F, 2.0F, 3.0F, 4.0F}));
  const auto progression_inspection = inspect_gguf_file(progression_path, GgufInspectionLimits{});
  const auto progression_chunk = selected_chunk(*progression_inspection.inventory);
  const auto first = scan_gguf_pattern_candidates(
      progression_path, *progression_inspection.inventory, {&progression_chunk, 1},
      {.max_payload_bytes = 16, .max_candidates = 8});
  const auto second = scan_gguf_pattern_candidates(
      progression_path, *progression_inspection.inventory, {&progression_chunk, 1},
      {.max_payload_bytes = 16, .max_candidates = 8});

  CHECK(repeated.status == Status::Confirmed);
  CHECK(has_kind(repeated, GgufPatternKind::RepeatedBytes));
  CHECK(first.status == Status::Confirmed);
  CHECK(has_kind(first, GgufPatternKind::ExactF32ArithmeticProgression));
  CHECK(first.trace->id == second.trace->id);
}

TEST(gguf_pattern_scan_stops_at_its_work_budget_and_never_modifies_input) {
  const auto bytes = f32_file({1.0F, 2.0F, 3.0F, 4.0F});
  const auto path = test::gguf::write_file("budget", bytes);
  const auto inspection = inspect_gguf_file(path, GgufInspectionLimits{});
  const auto chunk = selected_chunk(*inspection.inventory);

  const auto result = scan_gguf_pattern_candidates(
      path, *inspection.inventory, {&chunk, 1},
      {.max_payload_bytes = 4, .max_candidates = 8});

  CHECK(result.status == Status::BudgetExceeded);
  CHECK(test::gguf::read_file(path) == bytes);
}
