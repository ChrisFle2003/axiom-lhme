#include "command.hpp"

#include "axiom/lhme/gguf_report.hpp"

#include <charconv>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace axiom::lhme::tool {
namespace {

struct Arguments {
  std::filesystem::path model_path;
  std::uint64_t memory_budget_bytes = 0;
  std::uint64_t chunk_size_bytes = 0;
  GgufLoadingPolicy policy = GgufLoadingPolicy::Budgeted;
  std::optional<std::filesystem::path> use_profile_path;
  std::optional<std::filesystem::path> json_output_path;
  bool scan = false;
};

[[nodiscard]] bool parse_u64(std::string_view text, std::uint64_t& value) {
  const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
  return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

[[nodiscard]] bool parse_policy(std::string_view text, GgufLoadingPolicy& policy) {
  if (text == "all-resident") {
    policy = GgufLoadingPolicy::AllResident;
  } else if (text == "budgeted") {
    policy = GgufLoadingPolicy::Budgeted;
  } else if (text == "metadata-only") {
    policy = GgufLoadingPolicy::MetadataOnly;
  } else {
    return false;
  }
  return true;
}

[[nodiscard]] bool parse_arguments(std::span<const std::string_view> values,
                                   Arguments& parsed, std::ostream& error) {
  if (values.size() < 2U) {
    error << "usage: axiom_gguf_inspect MODEL.gguf --memory-budget BYTES "
             "--chunk-size BYTES --policy all-resident|budgeted|metadata-only\n";
    return false;
  }
  parsed.model_path = std::string(values[1]);
  bool has_budget = false;
  bool has_chunk_size = false;
  bool has_policy = false;
  for (std::size_t index = 2; index < values.size(); ++index) {
    const auto option = values[index];
    if (option == "--scan") {
      parsed.scan = true;
      continue;
    }
    if (index + 1U >= values.size()) {
      error << "missing value for " << option << '\n';
      return false;
    }
    const auto value = values[++index];
    if (option == "--memory-budget") {
      has_budget = parse_u64(value, parsed.memory_budget_bytes);
    } else if (option == "--chunk-size") {
      has_chunk_size = parse_u64(value, parsed.chunk_size_bytes) &&
                       parsed.chunk_size_bytes != 0;
    } else if (option == "--policy") {
      has_policy = parse_policy(value, parsed.policy);
    } else if (option == "--use-profile") {
      parsed.use_profile_path = std::string(value);
    } else if (option == "--json-output") {
      parsed.json_output_path = std::string(value);
    } else {
      error << "unknown option: " << option << '\n';
      return false;
    }
    if ((!has_budget && option == "--memory-budget") ||
        (!has_chunk_size && option == "--chunk-size") ||
        (!has_policy && option == "--policy")) {
      error << "invalid value for " << option << '\n';
      return false;
    }
  }
  if (!has_budget || !has_chunk_size || !has_policy) {
    error << "--memory-budget, --chunk-size and --policy are required\n";
    return false;
  }
  return true;
}

[[nodiscard]] bool read_profile(const std::filesystem::path& path,
                                std::vector<std::string>& names) {
  std::ifstream input(path);
  if (!input) return false;
  for (std::string line; std::getline(input, line);) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (!line.empty()) names.push_back(std::move(line));
  }
  return true;
}

}  // namespace

int run_gguf_inspect(std::span<const std::string_view> arguments,
                     std::istream& input, std::ostream& output,
                     std::ostream& error) {
  (void)input;
  Arguments arguments_result;
  if (!parse_arguments(arguments, arguments_result, error)) return 2;

  const auto inspection = inspect_gguf_file(arguments_result.model_path);
  if (inspection.status != Status::Confirmed || !inspection.inventory.has_value()) {
    error << "inspection failed: " << inspection.diagnostic << '\n';
    return inspection.status == Status::BudgetExceeded ? 4 : 3;
  }

  GgufPlanConfig plan_config{.memory_budget_bytes = arguments_result.memory_budget_bytes,
                             .chunk_size_bytes = arguments_result.chunk_size_bytes,
                             .policy = arguments_result.policy};
  if (arguments_result.use_profile_path.has_value() &&
      !read_profile(*arguments_result.use_profile_path,
                    plan_config.ordered_tensor_use_profile)) {
    error << "unable to read use profile\n";
    return 2;
  }
  const auto plan = make_gguf_storage_plan(*inspection.inventory, plan_config);
  if (plan.status == Status::InvalidArgument) {
    error << "planning failed: " << plan.diagnostic << '\n';
    return 2;
  }

  std::optional<GgufPatternScanResult> pattern_scan;
  int exit_code = plan.status == Status::BudgetExceeded ? 4 : 0;
  if (arguments_result.scan) {
    pattern_scan = scan_gguf_pattern_candidates(
        arguments_result.model_path, *inspection.inventory, plan.items,
        {.max_payload_bytes = arguments_result.memory_budget_bytes,
         .max_candidates = 128});
    if (pattern_scan->status == Status::BudgetExceeded) {
      exit_code = 4;
    } else if (pattern_scan->status == Status::InvalidArgument) {
      exit_code = 2;
    } else if (pattern_scan->status != Status::Confirmed) {
      exit_code = 3;
    }
  }
  output << render_gguf_text_report(inspection, plan,
                                    pattern_scan.has_value() ? &*pattern_scan : nullptr);
  if (arguments_result.json_output_path.has_value()) {
    std::ofstream json_output(*arguments_result.json_output_path, std::ios::trunc);
    if (!json_output) {
      error << "unable to write JSON output\n";
      return 5;
    }
    json_output << render_gguf_json_plan(inspection, plan,
                                         pattern_scan.has_value() ? &*pattern_scan : nullptr)
                << '\n';
  }
  return exit_code;
}

}  // namespace axiom::lhme::tool
