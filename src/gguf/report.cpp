#include "bruce/lhme/gguf_report.hpp"

#include <iomanip>
#include <sstream>
#include <string_view>

namespace bruce::lhme {
namespace {

[[nodiscard]] std::string_view status_name(Status status) {
  switch (status) {
    case Status::Confirmed: return "confirmed";
    case Status::Rejected: return "rejected";
    case Status::BudgetExceeded: return "budget_exceeded";
    case Status::Cancelled: return "cancelled";
    case Status::InvalidArgument: return "invalid_argument";
    case Status::UnsupportedMode: return "unsupported_mode";
    case Status::InternalError: return "internal_error";
  }
  return "internal_error";
}

[[nodiscard]] std::string_view placement_name(GgufPlacement placement) {
  switch (placement) {
    case GgufPlacement::Resident: return "resident";
    case GgufPlacement::OnDemand: return "on_demand";
    case GgufPlacement::Deferred: return "deferred";
    case GgufPlacement::Rejected: return "rejected";
  }
  return "rejected";
}

[[nodiscard]] std::string json_escape(std::string_view value) {
  std::ostringstream output;
  for (const unsigned char character : value) {
    switch (character) {
      case '"': output << "\\\""; break;
      case '\\': output << "\\\\"; break;
      case '\b': output << "\\b"; break;
      case '\f': output << "\\f"; break;
      case '\n': output << "\\n"; break;
      case '\r': output << "\\r"; break;
      case '\t': output << "\\t"; break;
      default:
        if (character < 0x20U) {
          output << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                 << static_cast<unsigned int>(character) << std::dec;
        } else {
          output << static_cast<char>(character);
        }
    }
  }
  return output.str();
}

}  // namespace

std::string render_gguf_text_report(const GgufInspection& inspection,
                                    const GgufStoragePlan& plan,
                                    const GgufPatternScanResult* pattern_scan) {
  std::ostringstream output;
  output << "inspection_status=" << status_name(inspection.status) << '\n';
  output << "validation_error=" << static_cast<std::uint32_t>(inspection.error) << '\n';
  output << "diagnostic=" << inspection.diagnostic << '\n';
  if (inspection.inventory.has_value()) {
    output << "parser_version=" << inspection.inventory->version << '\n';
    output << "input_fingerprint=" << inspection.inventory->input_fingerprint << '\n';
    output << "file_byte_count=" << inspection.inventory->file_byte_count << '\n';
  }
  output << "plan_status=" << status_name(plan.status) << '\n';
  output << "memory_budget_bytes=" << plan.memory_budget_bytes << '\n';
  output << "resident_ram_bytes=" << plan.resident_ram_bytes << '\n';
  output << "trace_id=" << (plan.trace == nullptr ? "" : plan.trace->id) << '\n';
  for (const auto& item : plan.items) {
    output << "item tensor=" << item.tensor_name << " chunk=" << item.chunk_index
           << " file_offset=" << item.file_offset << " stored_byte_count="
           << item.stored_byte_count << " decoded_ram_bytes=" << item.decoded_ram_bytes
           << " placement=" << placement_name(item.placement)
           << " reason=" << item.reason << '\n';
  }
  if (pattern_scan != nullptr) {
    output << "pattern_scan_status=" << status_name(pattern_scan->status) << '\n';
    output << "pattern_candidate_count=" << pattern_scan->candidates.size() << '\n';
  }
  return output.str();
}

std::string render_gguf_json_plan(const GgufInspection& inspection,
                                  const GgufStoragePlan& plan,
                                  const GgufPatternScanResult* pattern_scan) {
  std::ostringstream output;
  output << "{\"inspection_status\":\"" << status_name(inspection.status)
         << "\",\"validation_error\":" << static_cast<std::uint32_t>(inspection.error)
         << ",\"diagnostic\":\"" << json_escape(inspection.diagnostic) << "\"";
  if (inspection.inventory.has_value()) {
    output << ",\"parser_version\":" << inspection.inventory->version
           << ",\"input_fingerprint\":\""
           << json_escape(inspection.inventory->input_fingerprint)
           << "\",\"file_byte_count\":" << inspection.inventory->file_byte_count;
  }
  output << ",\"plan_status\":\"" << status_name(plan.status)
         << "\",\"memory_budget_bytes\":" << plan.memory_budget_bytes
         << ",\"resident_ram_bytes\":" << plan.resident_ram_bytes
         << ",\"trace_id\":\""
         << json_escape(plan.trace == nullptr ? "" : plan.trace->id) << "\",\"items\":[";
  for (std::size_t index = 0; index < plan.items.size(); ++index) {
    const auto& item = plan.items[index];
    if (index != 0) output << ',';
    output << "{\"tensor_name\":\"" << json_escape(item.tensor_name)
           << "\",\"chunk_index\":" << item.chunk_index
           << ",\"file_offset\":" << item.file_offset
           << ",\"stored_byte_count\":" << item.stored_byte_count
           << ",\"decoded_ram_bytes\":" << item.decoded_ram_bytes
           << ",\"placement\":\"" << placement_name(item.placement)
           << "\",\"reason\":\"" << json_escape(item.reason) << "\"}";
  }
  output << ']';
  if (pattern_scan != nullptr) {
    output << ",\"pattern_scan_status\":\"" << status_name(pattern_scan->status)
           << "\",\"pattern_candidate_count\":" << pattern_scan->candidates.size();
  }
  output << '}';
  return output.str();
}

}  // namespace bruce::lhme
