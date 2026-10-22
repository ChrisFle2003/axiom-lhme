#include "test.hpp"

#include "axiom/lhme/formula.hpp"

namespace {

axiom::lhme::FormulaSpec reference_spec() {
  return axiom::lhme::make_power_offset_spec(2, 100, 1);
}

axiom::lhme::EvaluationResult exact(const axiom::lhme::FormulaSpec& spec,
                                    std::int64_t index) {
  return axiom::lhme::evaluate_power_offset(
      spec, {.index = index}, axiom::lhme::EvaluationMode::Exact,
      axiom::lhme::Budget::unlimited());
}

}  // namespace

TEST(power_offset_exact_reference_values) {
  const auto spec = reference_spec();
  CHECK(exact(spec, 0).exact_integer() == 0);
  CHECK(exact(spec, 1).exact_integer() == 100);
  CHECK(exact(spec, 5).exact_integer() == 3100);
  CHECK(exact(spec, 10).exact_integer() == 102300);
  CHECK(exact(spec, 20).exact_integer() == 104857500);
}

TEST(power_offset_rejects_negative_address) {
  const auto result = exact(reference_spec(), -1);
  CHECK(result.status == axiom::lhme::Status::InvalidArgument);
}
