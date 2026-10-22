#include "axiom/lhme/gguf_pattern_scan.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <limits>
#include <map>
#include <utility>

namespace axiom::lhme {
namespace {

constexpr std::uint32_t kGgmlTypeF32 = 0;

[[nodiscard]] bool checked_add(std::uint64_t left, std::uint64_t right,
                               std::uint64_t& result) {
  if (left > std::numeric_limits<std::uint64_t>::max() - right) return false;
  result = left + right;
  return true;
}

[[nodiscard]] GgufPatternScanResult result_with(Status status,
                                                std::string diagnostic) {
  return {.status = status,
          .diagnostic = std::move(diagnostic),
          .candidates = {},
          .trace = make_trace({{.scope = GateScope::Run,
                                .status = status,
                                .reason = "gguf pattern scan"}})};
}

[[nodiscard]] bool all_equal(std::span<const std::byte> bytes) {
  return !bytes.empty() &&
         std::all_of(bytes.begin() + 1, bytes.end(),
                     [first = bytes.front()](std::byte value) { return value == first; });
}

[[nodiscard]] bool repeated_unit(std::span<const std::byte> bytes,
                                 std::size_t unit_size) {
  if (unit_size == 0 || bytes.size() <= unit_size || bytes.size() % unit_size != 0U) {
    return false;
  }
  for (std::size_t offset = unit_size; offset < bytes.size(); ++offset) {
    if (bytes[offset] != bytes[offset % unit_size]) return false;
  }
  return true;
}

[[nodiscard]] std::uint32_t read_f32_bits(const std::byte* bytes) {
  std::uint32_t value = 0;
  for (std::size_t index = 0; index < sizeof(value); ++index) {
    value |= static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[index]))
             << (index * 8U);
  }
  return value;
}

[[nodiscard]] bool f32_progression(std::span<const std::byte> bytes) {
  if (bytes.size() < 3U * sizeof(float) || bytes.size() % sizeof(float) != 0U) {
    return false;
  }
  const auto start = std::bit_cast<float>(read_f32_bits(bytes.data()));
  const auto second = std::bit_cast<float>(read_f32_bits(bytes.data() + sizeof(float)));
  if (!std::isfinite(start) || !std::isfinite(second)) return false;
  const auto step = second - start;
  for (std::size_t index = 2; index < bytes.size() / sizeof(float); ++index) {
    const auto actual_bits = read_f32_bits(bytes.data() + index * sizeof(float));
    const auto expected = start + static_cast<float>(index) * step;
    if (!std::isfinite(expected) || std::bit_cast<std::uint32_t>(expected) != actual_bits) {
      return false;
    }
  }
  return true;
}

[[nodiscard]] bool f32_constant(std::span<const std::byte> bytes) {
  if (bytes.empty() || bytes.size() % sizeof(float) != 0U) return false;
  const auto first = read_f32_bits(bytes.data());
  for (std::size_t index = 1; index < bytes.size() / sizeof(float); ++index) {
    if (read_f32_bits(bytes.data() + index * sizeof(float)) != first) return false;
  }
  return true;
}

[[nodiscard]] bool append_candidate(std::vector<GgufPatternCandidate>& candidates,
                                    std::uint64_t max_candidates,
                                    GgufPatternCandidate candidate) {
  if (candidates.size() >= max_candidates) return false;
  candidates.push_back(std::move(candidate));
  return true;
}

}  // namespace

GgufPatternScanResult scan_gguf_pattern_candidates(
    const std::filesystem::path& path, const GgufInventory& inventory,
    std::span<const GgufPlanItem> selected_chunks,
    const GgufPatternScanConfig& config) {
  if (config.max_payload_bytes == 0 || config.max_candidates == 0) {
    return result_with(Status::InvalidArgument, "scan limits must be nonzero");
  }

  std::map<std::string, const GgufTensor*> tensors;
  for (const auto& tensor : inventory.tensors) {
    if (!tensors.emplace(tensor.name, &tensor).second) {
      return result_with(Status::InvalidArgument, "inventory tensor names must be unique");
    }
  }
  std::uint64_t requested_bytes = 0;
  for (const auto& chunk : selected_chunks) {
    const auto tensor = tensors.find(chunk.tensor_name);
    std::uint64_t chunk_end = 0;
    std::uint64_t tensor_end = 0;
    if (tensor == tensors.end() || chunk.stored_byte_count == 0 ||
        !checked_add(chunk.file_offset, chunk.stored_byte_count, chunk_end) ||
        !checked_add(tensor->second->absolute_file_offset,
                     tensor->second->stored_byte_count, tensor_end) ||
        chunk.file_offset < tensor->second->absolute_file_offset || chunk_end > tensor_end ||
        !checked_add(requested_bytes, chunk.stored_byte_count, requested_bytes)) {
      return result_with(Status::InvalidArgument, "selected chunk is outside inventory");
    }
  }
  if (requested_bytes > config.max_payload_bytes) {
    return result_with(Status::BudgetExceeded, "scan payload budget exceeded");
  }

  std::ifstream input(path, std::ios::binary);
  if (!input) return result_with(Status::Rejected, "unable to open GGUF input");

  GgufPatternScanResult result;
  result.status = Status::Confirmed;
  std::vector<GateEvent> events;
  events.push_back({.scope = GateScope::Run,
                    .status = Status::Confirmed,
                    .reason = "gguf pattern scan confirmed"});
  for (const auto& chunk : selected_chunks) {
    const auto* tensor = tensors.at(chunk.tensor_name);
    std::vector<std::byte> payload(static_cast<std::size_t>(chunk.stored_byte_count));
    input.seekg(static_cast<std::streamoff>(chunk.file_offset), std::ios::beg);
    input.read(reinterpret_cast<char*>(payload.data()),
               static_cast<std::streamsize>(payload.size()));
    if (!input) return result_with(Status::Rejected, "unable to read selected chunk");

    const auto bytes = std::span<const std::byte>{payload};
    const auto base_candidate = [&chunk]() {
      return GgufPatternCandidate{.tensor_name = chunk.tensor_name,
                                  .file_offset = chunk.file_offset,
                                  .stored_byte_count = chunk.stored_byte_count,
                                  .exactly_verified = false};
    };
    const auto constant = all_equal(bytes) ||
                          (tensor->ggml_type == kGgmlTypeF32 && f32_constant(bytes));
    if (constant && !append_candidate(result.candidates, config.max_candidates,
                                      {.kind = GgufPatternKind::ConstantBytes,
                                       .tensor_name = base_candidate().tensor_name,
                                       .file_offset = base_candidate().file_offset,
                                       .stored_byte_count = base_candidate().stored_byte_count,
                                       .exactly_verified = false,
                                       .evidence = "constant byte or F32 element sequence"})) {
      return result_with(Status::BudgetExceeded, "scan candidate budget exceeded");
    }
    for (std::size_t unit_size = 1; unit_size <= bytes.size() / 2U; ++unit_size) {
      if (repeated_unit(bytes, unit_size)) {
        if (!append_candidate(result.candidates, config.max_candidates,
                              {.kind = GgufPatternKind::RepeatedBytes,
                               .tensor_name = chunk.tensor_name,
                               .file_offset = chunk.file_offset,
                               .stored_byte_count = chunk.stored_byte_count,
                               .exactly_verified = false,
                               .evidence = "repeated raw byte block"})) {
          return result_with(Status::BudgetExceeded, "scan candidate budget exceeded");
        }
        break;
      }
    }
    if (tensor->ggml_type == kGgmlTypeF32 && f32_progression(bytes) &&
        !append_candidate(result.candidates, config.max_candidates,
                          {.kind = GgufPatternKind::ExactF32ArithmeticProgression,
                           .tensor_name = chunk.tensor_name,
                           .file_offset = chunk.file_offset,
                           .stored_byte_count = chunk.stored_byte_count,
                           .exactly_verified = false,
                           .evidence = "bit-exact F32 arithmetic progression candidate"})) {
      return result_with(Status::BudgetExceeded, "scan candidate budget exceeded");
    }
    events.push_back({.scope = GateScope::Tile,
                      .status = Status::Confirmed,
                      .reason = "selected chunk scanned"});
  }
  result.trace = make_trace(std::move(events));
  return result;
}

}  // namespace axiom::lhme
