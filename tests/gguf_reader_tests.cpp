#include "test.hpp"

#include "gguf_fixture.hpp"

#include "bruce/lhme/gguf_inspector.hpp"

#include <cstddef>
#include <array>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <vector>

namespace {

using bruce::lhme::GgufInspectionLimits;
using bruce::lhme::GgufValidationError;
using bruce::lhme::Status;
using bruce::lhme::inspect_gguf_file;

void expect_rejected(const std::filesystem::path& path,
                     GgufValidationError expected_error) {
  const auto inspection = inspect_gguf_file(path, GgufInspectionLimits{});
  CHECK(inspection.status == Status::Rejected);
  CHECK(inspection.error == expected_error);
}

}  // namespace

TEST(gguf_reader_inventories_a_valid_v3_f32_file_without_modifying_it) {
  const auto bytes = test::gguf::valid_f32_file();
  const auto path = test::gguf::write_file("valid", bytes);

  const auto first = inspect_gguf_file(path, GgufInspectionLimits{});
  const auto second = inspect_gguf_file(path, GgufInspectionLimits{});

  CHECK(first.status == Status::Confirmed);
  CHECK(first.inventory.has_value());
  CHECK(first.inventory->version == test::gguf::kVersion);
  CHECK(first.inventory->input_fingerprint == second.inventory->input_fingerprint);
  CHECK(first.inventory->tensors.size() == 1U);
  CHECK(first.inventory->tensors[0].name == "weight");
  CHECK(first.inventory->tensors[0].shape == std::vector<std::uint64_t>{2});
  CHECK(first.inventory->tensors[0].logical_element_count == 2U);
  CHECK(first.inventory->tensors[0].stored_byte_count == 8U);
  CHECK(first.inventory->tensors[0].absolute_file_offset % 32U == 0U);
  CHECK(first.trace->id == second.trace->id);
  CHECK(test::gguf::read_file(path) == bytes);
}

TEST(gguf_reader_rejects_invalid_magic_before_reading_descriptors) {
  auto bytes = test::gguf::valid_f32_file();
  bytes[0] = std::byte{0};
  expect_rejected(test::gguf::write_file("magic", bytes),
                  GgufValidationError::InvalidMagic);
}

TEST(gguf_reader_rejects_an_unsupported_version_before_reading_descriptors) {
  auto bytes = test::gguf::valid_f32_file();
  bytes[4] = std::byte{4};
  expect_rejected(test::gguf::write_file("version", bytes),
                  GgufValidationError::UnsupportedVersion);
}

TEST(gguf_reader_rejects_truncated_metadata_string_without_allocating_declared_length) {
  expect_rejected(test::gguf::write_file("metadata-string",
                                         test::gguf::truncated_metadata_string_file()),
                  GgufValidationError::TruncatedInput);
}

TEST(gguf_reader_rejects_truncated_metadata_array_without_allocating_declared_length) {
  expect_rejected(test::gguf::write_file("metadata-array",
                                         test::gguf::truncated_metadata_array_file()),
                  GgufValidationError::TruncatedInput);
}

TEST(gguf_reader_rejects_dimension_products_that_overflow) {
  const auto bytes = test::gguf::valid_f32_file(
      {std::numeric_limits<std::uint64_t>::max(), 2U}, 0U, false);
  expect_rejected(test::gguf::write_file("dimension-overflow", bytes),
                  GgufValidationError::IntegerOverflow);
}

TEST(gguf_reader_rejects_tensor_ranges_that_overflow) {
  const auto bytes = test::gguf::valid_f32_file(
      {2U}, std::numeric_limits<std::uint64_t>::max() - 31U, false);
  expect_rejected(test::gguf::write_file("range-overflow", bytes),
                  GgufValidationError::IntegerOverflow);
}

TEST(gguf_reader_rejects_misaligned_tensor_offsets) {
  const auto bytes = test::gguf::valid_f32_file({2U}, 1U);
  expect_rejected(test::gguf::write_file("misaligned", bytes),
                  GgufValidationError::MisalignedTensorOffset);
}

TEST(gguf_reader_accounts_for_each_supported_v3_tensor_layout) {
  struct TypeCase {
    std::uint32_t type;
    std::uint64_t elements_per_block;
    std::uint64_t bytes_per_block;
  };
  constexpr std::array<TypeCase, 14> cases = {{
      {test::gguf::kTypeF32, 1, 4},   {test::gguf::kTypeF16, 1, 2},
      {test::gguf::kTypeQ40, 32, 18}, {test::gguf::kTypeQ41, 32, 20},
      {test::gguf::kTypeQ50, 32, 22}, {test::gguf::kTypeQ51, 32, 24},
      {test::gguf::kTypeQ80, 32, 34}, {test::gguf::kTypeQ81, 32, 40},
      {test::gguf::kTypeQ2K, 256, 84}, {test::gguf::kTypeQ3K, 256, 110},
      {test::gguf::kTypeQ4K, 256, 144}, {test::gguf::kTypeQ5K, 256, 176},
      {test::gguf::kTypeQ6K, 256, 210}, {test::gguf::kTypeQ8K, 256, 292},
  }};

  for (const auto type_case : cases) {
    const auto path = test::gguf::write_file(
        "layout", test::gguf::tensor_file({type_case.elements_per_block},
                                             type_case.type,
                                             type_case.bytes_per_block));
    const auto inspection = inspect_gguf_file(path, GgufInspectionLimits{});

    CHECK(inspection.status == Status::Confirmed);
    CHECK(inspection.inventory->tensors.size() == 1U);
    const auto& tensor = inspection.inventory->tensors.front();
    CHECK(tensor.logical_element_count == type_case.elements_per_block);
    CHECK(tensor.stored_byte_count == type_case.bytes_per_block);
    CHECK(tensor.decoded_ram_bytes == type_case.elements_per_block * 4U);
  }
}

TEST(gguf_reader_rejects_a_tensor_payload_smaller_than_its_quantized_layout) {
  const auto path = test::gguf::write_file(
      "short-q40", test::gguf::tensor_file({32U}, test::gguf::kTypeQ40, 17U));

  expect_rejected(path, GgufValidationError::TensorRangeOutOfBounds);
}

TEST(gguf_reader_rejects_quantized_tensor_sizes_that_do_not_fill_a_block) {
  const auto path = test::gguf::write_file(
      "partial-q40", test::gguf::tensor_file({31U}, test::gguf::kTypeQ40, 18U));

  expect_rejected(path, GgufValidationError::InvalidTensorDimension);
}
