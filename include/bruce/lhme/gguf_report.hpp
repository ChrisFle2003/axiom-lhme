#pragma once

#include <string>

#include "bruce/lhme/gguf_inspector.hpp"
#include "bruce/lhme/gguf_pattern_scan.hpp"
#include "bruce/lhme/gguf_storage_plan.hpp"

namespace bruce::lhme {

[[nodiscard]] std::string render_gguf_text_report(
    const GgufInspection& inspection, const GgufStoragePlan& plan,
    const GgufPatternScanResult* pattern_scan = nullptr);

[[nodiscard]] std::string render_gguf_json_plan(
    const GgufInspection& inspection, const GgufStoragePlan& plan,
    const GgufPatternScanResult* pattern_scan = nullptr);

}  // namespace bruce::lhme
