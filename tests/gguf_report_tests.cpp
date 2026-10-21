#include "test.hpp"

#include "gguf_fixture.hpp"

#include "command.hpp"
#include "bruce/lhme/gguf_report.hpp"

#include <array>
#include <sstream>
#include <string>
#include <string_view>

namespace {

using bruce::lhme::GgufInspectionLimits;
using bruce::lhme::GgufLoadingPolicy;
using bruce::lhme::GgufPlanConfig;
using bruce::lhme::Status;
using bruce::lhme::inspect_gguf_file;
using bruce::lhme::make_gguf_storage_plan;
using bruce::lhme::render_gguf_json_plan;
using bruce::lhme::render_gguf_text_report;

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
  const std::array<std::string_view, 2> arguments = {"bruce_gguf_inspect", "model.gguf"};
  std::istringstream input;
  std::ostringstream output;
  std::ostringstream error;

  const auto exit_code = bruce::lhme::tool::run_gguf_inspect(arguments, input, output, error);

  CHECK(exit_code != 0);
  CHECK(!error.str().empty());
}
