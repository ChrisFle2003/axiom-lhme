#include "test.hpp"

#include "gguf_fixture.hpp"

#include "command.hpp"
#include "axiom/lhme/gguf_report.hpp"
#include "axiom/lhme/trace.hpp"

#include <array>
#include <sstream>
#include <string>
#include <string_view>

namespace {

using axiom::lhme::GgufInspectionLimits;
using axiom::lhme::GgufLoadingPolicy;
using axiom::lhme::GgufPlanConfig;
using axiom::lhme::Status;
using axiom::lhme::inspect_gguf_file;
using axiom::lhme::make_gguf_storage_plan;
using axiom::lhme::render_gguf_json_plan;
using axiom::lhme::render_gguf_text_report;

}  // namespace

TEST(gguf_report_includes_stable_inventory_plan_and_trace_fields) {
  const auto path = test::gguf::write_file("report", test::gguf::valid_f32_file());
  const auto inspection = inspect_gguf_file(path, GgufInspectionLimits{});
  const auto plan = make_gguf_storage_plan(
      *inspection.inventory, {.memory_budget_bytes = 8,
                              .chunk_size_bytes = 8,
                              .policy = GgufLoadingPolicy::AllResident});

  const auto text = render_gguf_text_report(inspection, plan);
  const auto first_json = render_gguf_json_plan(inspection, plan);
  const auto second_json = render_gguf_json_plan(inspection, plan);

  CHECK(inspection.status == Status::Confirmed);
  CHECK(plan.status == Status::Confirmed);
  CHECK(text.find(inspection.inventory->input_fingerprint) != std::string::npos);
  CHECK(text.find("parser_version=3") != std::string::npos);
  CHECK(text.find("placement=resident") != std::string::npos);
  CHECK(text.find(plan.trace->id) != std::string::npos);
  CHECK(first_json == second_json);
  CHECK(first_json.find("\"input_fingerprint\"") != std::string::npos);
  CHECK(first_json.find("\"memory_budget_bytes\":8") != std::string::npos);
  CHECK(first_json.find("\"placement\":\"resident\"") != std::string::npos);
}

TEST(gguf_command_rejects_missing_required_arguments) {
  const std::array<std::string_view, 2> arguments = {"axiom_gguf_inspect", "model.gguf"};
  std::istringstream input;
  std::ostringstream output;
  std::ostringstream error;

  const auto exit_code = axiom::lhme::tool::run_gguf_inspect(arguments, input, output, error);

  CHECK(exit_code != 0);
  CHECK(!error.str().empty());
}

TEST(gguf_command_returns_a_budget_exit_code_when_pattern_scan_exceeds_its_limit) {
  const auto path = test::gguf::write_file("scan-budget", test::gguf::valid_f32_file());
  const auto model_path = path.string();
  const std::array<std::string_view, 9> arguments = {
      "axiom_gguf_inspect", model_path, "--memory-budget", "4", "--chunk-size",
      "8", "--policy", "budgeted", "--scan"};
  std::istringstream input;
  std::ostringstream output;
  std::ostringstream error;

  const auto exit_code = axiom::lhme::tool::run_gguf_inspect(arguments, input, output, error);

  CHECK(exit_code == 4);
  CHECK(output.str().find("pattern_scan_status=budget_exceeded") != std::string::npos);
}

TEST(gguf_text_report_escapes_control_characters_from_tensor_names) {
  const axiom::lhme::GgufInspection inspection{
      .status = Status::Confirmed,
      .inventory = axiom::lhme::GgufInventory{.version = 3,
                                               .input_fingerprint = "fixture"}};
  const axiom::lhme::GgufStoragePlan plan{
      .status = Status::Confirmed,
      .items = {{.tensor_name = "weight\x1b[2J",
                 .placement = axiom::lhme::GgufPlacement::Deferred}},
      .trace = axiom::lhme::make_trace({})};

  const auto text = render_gguf_text_report(inspection, plan);

  CHECK(text.find('\x1b') == std::string::npos);
  CHECK(text.find("weight\\x1b[2J") != std::string::npos);
}
