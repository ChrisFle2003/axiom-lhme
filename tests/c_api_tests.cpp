#include "test.hpp"

#include <string>

#include "bruce/lhme/c_api.h"

TEST(c_api_uses_opaque_handles_and_releases_everything) {
  const lhme_engine_config config{.cache_max_entries = 8,
                                  .cache_max_bytes = 4096,
                                  .worker_count = 1,
                                  .tile_size = 4};
  lhme_engine* engine = lhme_engine_create(&config);
  CHECK(engine != nullptr);

  const lhme_power_offset_spec spec{.base_decimal = "2",
                                    .scale_decimal = "100",
                                    .version = 1};
  lhme_formula* formula = lhme_formula_register(engine, &spec);
  CHECK(formula != nullptr);

  const lhme_evaluation_request request{
      .index = 10,
      .mode = LHME_MODE_EXACT,
      .max_bit_length = UINT64_MAX,
      .modulus_decimal = nullptr};
  lhme_result* result = lhme_evaluate(engine, formula, &request);
  CHECK(result != nullptr);
  CHECK(lhme_result_status(result) == LHME_STATUS_CONFIRMED);
  CHECK(std::string(lhme_result_exact_decimal(result)) == "102300");

  lhme_result_destroy(result);
  lhme_formula_destroy(formula);
  lhme_engine_destroy(engine);
}

TEST(c_api_rejects_an_unknown_mode_instead_of_silently_changing_it) {
  lhme_engine* engine = lhme_engine_create(nullptr);
  const lhme_power_offset_spec spec{.base_decimal = "2",
                                    .scale_decimal = "100",
                                    .version = 1};
  lhme_formula* formula = lhme_formula_register(engine, &spec);
  const lhme_evaluation_request request{
      .index = 10,
      .mode = static_cast<lhme_mode>(99),
      .max_bit_length = UINT64_MAX,
      .modulus_decimal = nullptr};

  lhme_result* result = lhme_evaluate(engine, formula, &request);

  CHECK(result != nullptr);
  CHECK(lhme_result_status(result) == LHME_STATUS_INVALID_ARGUMENT);
  lhme_result_destroy(result);
  lhme_formula_destroy(formula);
  lhme_engine_destroy(engine);
}
