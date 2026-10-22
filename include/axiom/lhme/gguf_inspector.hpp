#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "axiom/lhme/trace.hpp"
#include "axiom/lhme/types.hpp"

namespace axiom::lhme {

enum class GgufValidationError {
  None,
  InvalidLimits,
  FileOpenFailed,
  FileSizeExceeded,
  InvalidMagic,
  UnsupportedVersion,
  TruncatedInput,
  InvalidMetadataType,
  InvalidCounts,
  InvalidTensorRank,
  InvalidTensorDimension,
  IntegerOverflow,
  UnsupportedTensorType,
  InvalidAlignment,
  MisalignedTensorOffset,
  TensorRangeOutOfBounds,
};

struct GgufInspectionLimits {
  std::uint64_t max_file_bytes = 1ULL << 30U;
  std::uint64_t max_metadata_entries = 1'000'000;
  std::uint64_t max_tensor_count = 1'000'000;
  std::uint64_t max_dimensions_per_tensor = 32;
  std::uint64_t max_string_bytes = 1ULL << 20U;
};

struct GgufTensor {
  std::string name;
  std::vector<std::uint64_t> shape;
  std::uint32_t ggml_type = 0;
  std::uint64_t logical_element_count = 0;
  std::uint64_t relative_data_offset = 0;
  std::uint64_t absolute_file_offset = 0;
  std::uint64_t stored_byte_count = 0;
  std::optional<std::uint64_t> decoded_ram_bytes;
};

struct GgufInventory {
  std::uint32_t version = 0;
  std::uint64_t alignment = 0;
  std::uint64_t file_byte_count = 0;
  std::string input_fingerprint;
  std::vector<GgufTensor> tensors;
};

struct GgufInspection {
  Status status = Status::InternalError;
  GgufValidationError error = GgufValidationError::None;
  std::string diagnostic;
  std::optional<GgufInventory> inventory;
  std::shared_ptr<const TraceRecord> trace;
};

[[nodiscard]] GgufInspection inspect_gguf_file(
    const std::filesystem::path& path,
    const GgufInspectionLimits& limits = {});

}  // namespace axiom::lhme
