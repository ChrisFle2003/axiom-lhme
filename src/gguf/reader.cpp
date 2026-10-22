#include "axiom/lhme/gguf_inspector.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string_view>
#include <utility>

namespace axiom::lhme {
namespace {

constexpr std::uint32_t kGgufMagic = 0x46554747U;
constexpr std::uint32_t kSupportedGgufVersion = 3;
constexpr std::uint64_t kDefaultAlignment = 32;

struct TensorLayout {
  std::uint32_t type;
  std::uint64_t elements_per_block;
  std::uint64_t bytes_per_block;
};

constexpr std::array<TensorLayout, 14> kSupportedTensorLayouts = {{
    {0, 1, 4},     {1, 1, 2},     {2, 32, 18},   {3, 32, 20},
    {6, 32, 22},   {7, 32, 24},   {8, 32, 34},   {9, 32, 40},
    {10, 256, 84}, {11, 256, 110}, {12, 256, 144}, {13, 256, 176},
    {14, 256, 210}, {15, 256, 292},
}};

enum class GgufValueType : std::uint32_t {
  Uint8 = 0,
  Int8 = 1,
  Uint16 = 2,
  Int16 = 3,
  Uint32 = 4,
  Int32 = 5,
  Float32 = 6,
  Bool = 7,
  String = 8,
  Array = 9,
  Uint64 = 10,
  Int64 = 11,
  Float64 = 12,
};

struct ParsedTensor {
  std::string name;
  std::vector<std::uint64_t> shape;
  std::uint32_t type = 0;
  std::uint64_t element_count = 0;
  std::uint64_t relative_offset = 0;
  std::uint64_t stored_bytes = 0;
  std::uint64_t decoded_ram_bytes = 0;
};

class BoundedReader {
 public:
  BoundedReader(const std::filesystem::path& path, std::uint64_t file_size)
      : input_(path, std::ios::binary), file_size_(file_size) {}

  [[nodiscard]] bool is_open() const { return input_.is_open(); }
  [[nodiscard]] std::uint64_t position() const { return position_; }
  [[nodiscard]] std::uint64_t remaining() const { return file_size_ - position_; }

  bool read_u32(std::uint32_t& value) {
    std::array<std::byte, 4> bytes{};
    if (!read_bytes(bytes)) return false;
    value = 0;
    for (std::size_t index = 0; index < bytes.size(); ++index) {
      value |= static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[index]))
               << (index * 8U);
    }
    return true;
  }

  bool read_u64(std::uint64_t& value) {
    std::array<std::byte, 8> bytes{};
    if (!read_bytes(bytes)) return false;
    value = 0;
    for (std::size_t index = 0; index < bytes.size(); ++index) {
      value |= static_cast<std::uint64_t>(std::to_integer<std::uint8_t>(bytes[index]))
               << (index * 8U);
    }
    return true;
  }

  bool read_string(std::string& value, std::uint64_t max_bytes) {
    std::uint64_t byte_count = 0;
    if (!read_u64(byte_count) || byte_count > max_bytes || byte_count > remaining() ||
        byte_count > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
      return false;
    }
    value.resize(static_cast<std::size_t>(byte_count));
    if (byte_count == 0) return true;
    input_.read(value.data(), static_cast<std::streamsize>(byte_count));
    if (!input_) return false;
    position_ += byte_count;
    return true;
  }

  bool skip(std::uint64_t byte_count) {
    if (byte_count > remaining()) return false;
    input_.seekg(static_cast<std::streamoff>(byte_count), std::ios::cur);
    if (!input_) return false;
    position_ += byte_count;
    return true;
  }

 private:
  template <std::size_t Size>
  bool read_bytes(std::array<std::byte, Size>& bytes) {
    if (Size > remaining()) return false;
    input_.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(Size));
    if (!input_) return false;
    position_ += Size;
    return true;
  }

  std::ifstream input_;
  std::uint64_t file_size_ = 0;
  std::uint64_t position_ = 0;
};

[[nodiscard]] bool checked_add(std::uint64_t left, std::uint64_t right,
                               std::uint64_t& result) {
  if (left > std::numeric_limits<std::uint64_t>::max() - right) return false;
  result = left + right;
  return true;
}

[[nodiscard]] bool checked_multiply(std::uint64_t left, std::uint64_t right,
                                    std::uint64_t& result) {
  if (left != 0 && right > std::numeric_limits<std::uint64_t>::max() / left) {
    return false;
  }
  result = left * right;
  return true;
}

[[nodiscard]] bool is_power_of_two(std::uint64_t value) {
  return value != 0 && (value & (value - 1U)) == 0;
}

[[nodiscard]] const TensorLayout* find_tensor_layout(std::uint32_t type) {
  const auto match = std::find_if(
      kSupportedTensorLayouts.begin(), kSupportedTensorLayouts.end(),
      [type](const TensorLayout& layout) { return layout.type == type; });
  return match == kSupportedTensorLayouts.end() ? nullptr : &*match;
}

[[nodiscard]] bool align_up(std::uint64_t value, std::uint64_t alignment,
                            std::uint64_t& result) {
  if (!is_power_of_two(alignment)) return false;
  const auto mask = alignment - 1U;
  if (value > std::numeric_limits<std::uint64_t>::max() - mask) return false;
  result = (value + mask) & ~mask;
  return true;
}

[[nodiscard]] std::optional<std::uint64_t> fixed_value_size(
    GgufValueType type) {
  switch (type) {
    case GgufValueType::Uint8:
    case GgufValueType::Int8:
    case GgufValueType::Bool:
      return 1;
    case GgufValueType::Uint16:
    case GgufValueType::Int16:
      return 2;
    case GgufValueType::Uint32:
    case GgufValueType::Int32:
    case GgufValueType::Float32:
      return 4;
    case GgufValueType::Uint64:
    case GgufValueType::Int64:
    case GgufValueType::Float64:
      return 8;
    case GgufValueType::String:
    case GgufValueType::Array:
      return std::nullopt;
  }
  return std::nullopt;
}

[[nodiscard]] bool is_known_value_type(std::uint32_t raw_type) {
  return raw_type <= static_cast<std::uint32_t>(GgufValueType::Float64);
}

[[nodiscard]] bool skip_metadata_value(BoundedReader& reader,
                                       std::uint32_t raw_type,
                                       const GgufInspectionLimits& limits) {
  if (!is_known_value_type(raw_type)) return false;
  const auto type = static_cast<GgufValueType>(raw_type);
  if (const auto size = fixed_value_size(type); size.has_value()) {
    return reader.skip(*size);
  }
  if (type == GgufValueType::String) {
    std::string ignored;
    return reader.read_string(ignored, limits.max_string_bytes);
  }

  std::uint32_t element_type = 0;
  std::uint64_t count = 0;
  if (!reader.read_u32(element_type) || !reader.read_u64(count) ||
      !is_known_value_type(element_type) ||
      element_type == static_cast<std::uint32_t>(GgufValueType::Array)) {
    return false;
  }
  const auto nested_type = static_cast<GgufValueType>(element_type);
  if (const auto element_size = fixed_value_size(nested_type); element_size.has_value()) {
    std::uint64_t total_size = 0;
    return checked_multiply(count, *element_size, total_size) && reader.skip(total_size);
  }
  for (std::uint64_t index = 0; index < count; ++index) {
    std::string ignored;
    if (!reader.read_string(ignored, limits.max_string_bytes)) return false;
  }
  return true;
}

[[nodiscard]] std::string fingerprint_file(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) return {};
  constexpr std::uint64_t kOffset = 14695981039346656037ULL;
  constexpr std::uint64_t kPrime = 1099511628211ULL;
  std::array<std::uint64_t, 4> states = {kOffset, kOffset ^ 1U, kOffset ^ 2U,
                                         kOffset ^ 3U};
  std::array<char, 4096> buffer{};
  while (input.read(buffer.data(), static_cast<std::streamsize>(buffer.size())) ||
         input.gcount() > 0) {
    for (std::streamsize index = 0; index < input.gcount(); ++index) {
      const auto value = static_cast<unsigned char>(buffer[static_cast<std::size_t>(index)]);
      for (std::size_t lane = 0; lane < states.size(); ++lane) {
        states[lane] ^= static_cast<std::uint64_t>(value + lane);
        states[lane] *= kPrime;
      }
    }
  }
  std::ostringstream output;
  output << std::hex << std::setfill('0');
  for (const auto state : states) output << std::setw(16) << state;
  return output.str();
}

[[nodiscard]] GgufInspection rejected(GgufValidationError error,
                                      std::string diagnostic,
                                      Status status = Status::Rejected) {
  return {.status = status,
          .error = error,
          .diagnostic = std::move(diagnostic),
          .inventory = std::nullopt,
          .trace = make_trace({{.scope = GateScope::Run,
                                .status = status,
                                .reason = "gguf inspection rejected"}})};
}

[[nodiscard]] bool valid_limits(const GgufInspectionLimits& limits) {
  return limits.max_file_bytes != 0 && limits.max_metadata_entries != 0 &&
         limits.max_tensor_count != 0 && limits.max_dimensions_per_tensor != 0 &&
         limits.max_string_bytes != 0;
}

}  // namespace

GgufInspection inspect_gguf_file(const std::filesystem::path& path,
                                 const GgufInspectionLimits& limits) {
  if (!valid_limits(limits)) {
    return rejected(GgufValidationError::InvalidLimits, "inspection limits must be nonzero",
                    Status::InvalidArgument);
  }

  std::error_code error_code;
  const auto file_size = std::filesystem::file_size(path, error_code);
  if (error_code) {
    return rejected(GgufValidationError::FileOpenFailed, "unable to determine input size");
  }
  if (file_size > limits.max_file_bytes) {
    return rejected(GgufValidationError::FileSizeExceeded, "input exceeds file-size limit",
                    Status::BudgetExceeded);
  }

  BoundedReader reader(path, file_size);
  if (!reader.is_open()) {
    return rejected(GgufValidationError::FileOpenFailed, "unable to open input file");
  }

  std::uint32_t magic = 0;
  std::uint32_t version = 0;
  std::uint64_t tensor_count = 0;
  std::uint64_t metadata_count = 0;
  if (!reader.read_u32(magic) || !reader.read_u32(version) ||
      !reader.read_u64(tensor_count) || !reader.read_u64(metadata_count)) {
    return rejected(GgufValidationError::TruncatedInput, "truncated GGUF header");
  }
  if (magic != kGgufMagic) {
    return rejected(GgufValidationError::InvalidMagic, "invalid GGUF magic");
  }
  if (version != kSupportedGgufVersion) {
    return rejected(GgufValidationError::UnsupportedVersion, "unsupported GGUF version");
  }
  if (tensor_count > limits.max_tensor_count ||
      metadata_count > limits.max_metadata_entries) {
    return rejected(GgufValidationError::InvalidCounts, "GGUF entry count exceeds limit",
                    Status::BudgetExceeded);
  }

  std::uint64_t alignment = kDefaultAlignment;
  for (std::uint64_t index = 0; index < metadata_count; ++index) {
    std::string key;
    std::uint32_t type = 0;
    if (!reader.read_string(key, limits.max_string_bytes) || !reader.read_u32(type)) {
      return rejected(GgufValidationError::TruncatedInput, "truncated GGUF metadata");
    }
    if (!is_known_value_type(type)) {
      return rejected(GgufValidationError::InvalidMetadataType,
                      "unsupported GGUF metadata type");
    }
    if (key == "general.alignment") {
      std::uint32_t alignment_value = 0;
      if (type != static_cast<std::uint32_t>(GgufValueType::Uint32) ||
          !reader.read_u32(alignment_value)) {
        return rejected(GgufValidationError::InvalidAlignment,
                        "general.alignment must be uint32");
      }
      alignment = alignment_value;
    } else if (!skip_metadata_value(reader, type, limits)) {
      return rejected(GgufValidationError::TruncatedInput, "truncated GGUF metadata value");
    }
  }
  if (!is_power_of_two(alignment)) {
    return rejected(GgufValidationError::InvalidAlignment,
                    "GGUF alignment must be a nonzero power of two");
  }

  std::vector<ParsedTensor> parsed_tensors;
  parsed_tensors.reserve(static_cast<std::size_t>(tensor_count));
  for (std::uint64_t index = 0; index < tensor_count; ++index) {
    ParsedTensor tensor;
    std::uint32_t rank = 0;
    if (!reader.read_string(tensor.name, limits.max_string_bytes) || !reader.read_u32(rank)) {
      return rejected(GgufValidationError::TruncatedInput, "truncated tensor descriptor");
    }
    if (rank > limits.max_dimensions_per_tensor) {
      return rejected(GgufValidationError::InvalidTensorRank, "tensor rank exceeds limit");
    }
    tensor.shape.reserve(rank);
    tensor.element_count = 1;
    for (std::uint32_t dimension_index = 0; dimension_index < rank; ++dimension_index) {
      std::uint64_t dimension = 0;
      if (!reader.read_u64(dimension)) {
        return rejected(GgufValidationError::TruncatedInput, "truncated tensor dimension");
      }
      if (dimension == 0) {
        return rejected(GgufValidationError::InvalidTensorDimension,
                        "tensor dimensions must be nonzero");
      }
      if (!checked_multiply(tensor.element_count, dimension, tensor.element_count)) {
        return rejected(GgufValidationError::IntegerOverflow,
                        "tensor element count overflows uint64");
      }
      tensor.shape.push_back(dimension);
    }
    if (!reader.read_u32(tensor.type) || !reader.read_u64(tensor.relative_offset)) {
      return rejected(GgufValidationError::TruncatedInput, "truncated tensor descriptor");
    }
    const auto* layout = find_tensor_layout(tensor.type);
    if (layout == nullptr) {
      return rejected(GgufValidationError::UnsupportedTensorType,
                      "unsupported GGML tensor type", Status::UnsupportedMode);
    }
    if (tensor.element_count % layout->elements_per_block != 0U) {
      return rejected(GgufValidationError::InvalidTensorDimension,
                      "tensor element count does not fill GGML blocks");
    }
    const auto block_count = tensor.element_count / layout->elements_per_block;
    if (!checked_multiply(block_count, layout->bytes_per_block, tensor.stored_bytes)) {
      return rejected(GgufValidationError::IntegerOverflow,
                      "tensor byte count overflows uint64");
    }
    if (!checked_multiply(tensor.element_count, 4U, tensor.decoded_ram_bytes)) {
      return rejected(GgufValidationError::IntegerOverflow,
                      "decoded tensor RAM estimate overflows uint64");
    }
    parsed_tensors.push_back(std::move(tensor));
  }

  std::uint64_t data_start = 0;
  if (!align_up(reader.position(), alignment, data_start)) {
    return rejected(GgufValidationError::IntegerOverflow, "tensor data start overflows uint64");
  }
  std::sort(parsed_tensors.begin(), parsed_tensors.end(),
            [](const ParsedTensor& left, const ParsedTensor& right) {
              return left.relative_offset == right.relative_offset
                         ? left.name < right.name
                         : left.relative_offset < right.relative_offset;
            });

  GgufInventory inventory;
  inventory.version = version;
  inventory.alignment = alignment;
  inventory.file_byte_count = file_size;
  inventory.input_fingerprint = fingerprint_file(path);
  if (inventory.input_fingerprint.empty()) {
    return rejected(GgufValidationError::FileOpenFailed, "unable to fingerprint input file");
  }
  inventory.tensors.reserve(parsed_tensors.size());
  std::uint64_t previous_end = data_start;
  for (const auto& tensor : parsed_tensors) {
    if (tensor.relative_offset % alignment != 0U) {
      return rejected(GgufValidationError::MisalignedTensorOffset,
                      "tensor offset is not aligned");
    }
    std::uint64_t absolute_offset = 0;
    std::uint64_t absolute_end = 0;
    if (!checked_add(data_start, tensor.relative_offset, absolute_offset) ||
        !checked_add(absolute_offset, tensor.stored_bytes, absolute_end)) {
      return rejected(GgufValidationError::IntegerOverflow, "tensor range overflows uint64");
    }
    if (absolute_end > file_size || absolute_offset < previous_end) {
      return rejected(GgufValidationError::TensorRangeOutOfBounds,
                      "tensor range is outside input data");
    }
    previous_end = absolute_end;
    inventory.tensors.push_back(
        {.name = tensor.name,
         .shape = tensor.shape,
         .ggml_type = tensor.type,
         .logical_element_count = tensor.element_count,
         .relative_data_offset = tensor.relative_offset,
         .absolute_file_offset = absolute_offset,
         .stored_byte_count = tensor.stored_bytes,
         .decoded_ram_bytes = tensor.decoded_ram_bytes});
  }

  const auto trace = make_trace({{.scope = GateScope::Run,
                                  .status = Status::Confirmed,
                                  .reason = "gguf v3 inventory confirmed"},
                                 {.scope = GateScope::Cache,
                                  .status = Status::Confirmed,
                                  .reason = "input-fingerprint=" +
                                            inventory.input_fingerprint}});
  return {.status = Status::Confirmed,
          .error = GgufValidationError::None,
          .diagnostic = "GGUF V3 inventory confirmed",
          .inventory = std::move(inventory),
          .trace = trace};
}

}  // namespace axiom::lhme
