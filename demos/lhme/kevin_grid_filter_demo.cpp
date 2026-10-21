#include "demo_report.hpp"

#include "bruce/lhme/domain_projection.hpp"
#include "bruce/lhme/kevin_filter.hpp"
#include "bruce/lhme/power_offset_kernel.hpp"

#include <memory>
#include <string>
#include <vector>

namespace {

class EngineTechniqueEvaluator final : public bruce::lhme::FormulaEvaluator {
 public:
  EngineTechniqueEvaluator(bruce::lhme::Engine& engine,
                           bruce::lhme::TechniqueIdentity identity,
                           std::string parameters)
      : engine_(engine), identity_(std::move(identity)), parameters_(std::move(parameters)) {}

  bruce::lhme::EvaluationResult evaluate(
      bruce::lhme::FormulaAddress address, bruce::lhme::EvaluationMode mode,
      bruce::lhme::Budget budget,
      std::optional<bruce::lhme::BigInt> modulus) override {
    return engine_.evaluate_technique({.identity = identity_,
                                       .parameters = parameters_,
                                       .address = address,
                                       .mode = mode,
                                       .budget = budget,
                                       .modulus = std::move(modulus)});
  }

 private:
  bruce::lhme::Engine& engine_;
  bruce::lhme::TechniqueIdentity identity_;
  std::string parameters_;
};

void add_tile(bruce::lhme::TileSummary& total,
              const bruce::lhme::TileSummary& tile) {
  total.status = tile.status;
  total.checked += tile.checked;
  total.modular_survivors += tile.modular_survivors;
  total.metric_survivors += tile.metric_survivors;
  total.final_survivors += tile.final_survivors;
  total.candidates.insert(total.candidates.end(), tile.candidates.begin(),
                          tile.candidates.end());
}

}  // namespace

int main() {
  using namespace bruce::lhme;

  demo::Report report;
  report.heading("KEVIN Domain Filter");

  auto engine = Engine::create({.cache_max_entries = 128, .cache_max_bytes = 65'536});
  report.expect_true("PowerOffset kernel registered",
                     engine.register_technique(std::make_shared<PowerOffsetKernel>()));

  const TechniqueIdentity technique{.id = "bruce.power_offset", .version = 1};
  const auto parameters = encode_power_offset_parameters(2, 100);
  EngineTechniqueEvaluator evaluator{engine, technique, parameters};
  const GridDomainSpec domain{{.id = "bruce.demo.grid-filter", .version = 1}, 2};
  DepthPathProjection projection;
  const DomainEvaluationContext context{.domain = domain, .projection = &projection,
                                        .evaluator = evaluator};

  const auto root = *GridAddress::from_steps({});
  const std::vector<GridStep> nested_steps{{.x = 1, .y = 0}};
  const auto nested = *GridAddress::from_steps(nested_steps);
  const auto first = filter_tile(context, root, /*modulus=*/17, /*residue=*/11,
                                 /*minimum bits=*/7, /*maximum bits=*/17);
  const auto second = filter_tile(context, nested, /*modulus=*/17, /*residue=*/11,
                                  /*minimum bits=*/7, /*maximum bits=*/17);
  TileSummary summary{.checked = 0};
  add_tile(summary, first);
  add_tile(summary, second);

  report.expect_equal("two tiles have 128 candidates", std::uint32_t(128), summary.checked);
  report.expect_equal("domain filter status", Status::Confirmed, summary.status);
  report.expect_equal("no exact work during filtering", std::uint32_t(0),
                      summary.exact_evaluations);
  report.expect_equal("reported survivors match addresses",
                      static_cast<std::uint32_t>(summary.candidates.size()),
                      summary.final_survivors);

  const auto exact = evaluate_survivors_exact(context, summary,
                                               /*candidate limit=*/1,
                                               Budget::unlimited());
  report.expect_true("exact phase evaluates at most one candidate", exact.values.size() <= 1);
  report.expect_equal("exact evaluation count matches values",
                      static_cast<std::uint32_t>(exact.values.size()),
                      exact.summary.exact_evaluations);

  report.row("tiles", "2");
  report.row("projection", projection.identity().id);
  report.row("modular survivors", std::to_string(summary.modular_survivors));
  report.row("metric survivors", std::to_string(summary.metric_survivors));
  report.row("final survivors", std::to_string(summary.final_survivors));
  report.row("exact evaluations", std::to_string(exact.summary.exact_evaluations));
  return report.finish();
}
