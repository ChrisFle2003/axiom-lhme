#include "test.hpp"

#include "bruce/lhme/formula.hpp"

namespace {

bruce::lhme::FormulaSpec reference_spec() {
  return bruce::lhme::make_power_offset_spec(2, 100, 1);
}

bruce::lhme::EvaluationResult exact(const bruce::lhme::FormulaSpec& spec,
                                    std::int64_t index) {
  return bruce::lhme::evaluate_power_offset(
      spec, {.index = index}, bruce::lhme::EvaluationMode::Exact,
      bruce::lhme::Budget::unlimited());
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
  CHECK(result.status == bruce::lhme::Status::InvalidArgument);
}
