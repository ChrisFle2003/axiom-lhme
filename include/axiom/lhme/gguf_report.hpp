#pragma once

#include <string>

#include "axiom/lhme/gguf_inspector.hpp"
#include "axiom/lhme/gguf_pattern_scan.hpp"
#include "axiom/lhme/gguf_storage_plan.hpp"

namespace axiom::lhme {

[[nodiscard]] std::string render_gguf_text_report(
    const GgufInspection& inspection, const GgufStoragePlan& plan,
    const GgufPatternScanResult* pattern_scan = nullptr);

[[nodiscard]] std::string render_gguf_json_plan(
    const GgufInspection& inspection, const GgufStoragePlan& plan,
    const GgufPatternScanResult* pattern_scan = nullptr);

}  // namespace axiom::lhme
