#include "test.hpp"

#include <string>

#include "axiom/lhme/axiom_lhme_c_api.h"

TEST(c_api_uses_opaque_handles_and_releases_everything) {
  const axiom_lhme_engine_config config{.cache_max_entries = 8,
                                  .cache_max_bytes = 4096,
                                  .worker_count = 1,
                                  .tile_size = 4};
  axiom_lhme_engine* engine = axiom_lhme_engine_create(&config);
  CHECK(engine != nullptr);

  const axiom_lhme_power_offset_spec spec{.base_decimal = "2",
                                    .scale_decimal = "100",
                                    .version = 1};
  axiom_lhme_formula* formula = axiom_lhme_formula_register(engine, &spec);
  CHECK(formula != nullptr);

  const axiom_lhme_evaluation_request request{
      .index = 10,
      .mode = AXIOM_LHME_MODE_EXACT,
      .max_bit_length = UINT64_MAX,
      .modulus_decimal = nullptr};
  axiom_lhme_result* result = axiom_lhme_evaluate(engine, formula, &request);
  CHECK(result != nullptr);
  CHECK(axiom_lhme_result_status(result) == AXIOM_LHME_STATUS_CONFIRMED);
  CHECK(std::string(axiom_lhme_result_exact_decimal(result)) == "102300");

  axiom_lhme_result_destroy(result);
  axiom_lhme_formula_destroy(formula);
  axiom_lhme_engine_destroy(engine);
}

TEST(c_api_rejects_an_unknown_mode_instead_of_silently_changing_it) {
  axiom_lhme_engine* engine = axiom_lhme_engine_create(nullptr);
  const axiom_lhme_power_offset_spec spec{.base_decimal = "2",
                                    .scale_decimal = "100",
                                    .version = 1};
  axiom_lhme_formula* formula = axiom_lhme_formula_register(engine, &spec);
  const axiom_lhme_evaluation_request request{
      .index = 10,
      .mode = static_cast<axiom_lhme_mode>(99),
      .max_bit_length = UINT64_MAX,
      .modulus_decimal = nullptr};

  axiom_lhme_result* result = axiom_lhme_evaluate(engine, formula, &request);

  CHECK(result != nullptr);
  CHECK(axiom_lhme_result_status(result) == AXIOM_LHME_STATUS_INVALID_ARGUMENT);
  axiom_lhme_result_destroy(result);
  axiom_lhme_formula_destroy(formula);
  axiom_lhme_engine_destroy(engine);
}
