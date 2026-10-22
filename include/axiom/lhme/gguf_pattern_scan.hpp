#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "axiom/lhme/gguf_inspector.hpp"
#include "axiom/lhme/gguf_storage_plan.hpp"
#include "axiom/lhme/trace.hpp"
#include "axiom/lhme/types.hpp"

namespace axiom::lhme {

enum class GgufPatternKind {
  ConstantBytes,
  RepeatedBytes,
  ExactF32ArithmeticProgression,
};

struct GgufPatternScanConfig {
  std::uint64_t max_payload_bytes = 0;
  std::uint64_t max_candidates = 0;
};

struct GgufPatternCandidate {
  GgufPatternKind kind = GgufPatternKind::ConstantBytes;
  std::string tensor_name;
  std::uint64_t file_offset = 0;
  std::uint64_t stored_byte_count = 0;
  bool exactly_verified = false;
  std::string evidence;
};

struct GgufPatternScanResult {
  Status status = Status::InternalError;
  std::string diagnostic;
  std::vector<GgufPatternCandidate> candidates;
  std::shared_ptr<const TraceRecord> trace;
};

[[nodiscard]] GgufPatternScanResult scan_gguf_pattern_candidates(
    const std::filesystem::path& path, const GgufInventory& inventory,
    std::span<const GgufPlanItem> selected_chunks,
    const GgufPatternScanConfig& config);

}  // namespace axiom::lhme
