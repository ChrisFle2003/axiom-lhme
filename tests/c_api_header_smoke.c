#include <stdint.h>
#include <string.h>

#include "bruce/lhme/c_api.h"

int main(void) {
  const lhme_engine_config config = {8, 4096, 1, 4};
  lhme_engine* engine = lhme_engine_create(&config);
  if (engine == NULL) return 1;

  const lhme_power_offset_spec spec = {"2", "100", 1};
  lhme_formula* formula = lhme_formula_register(engine, &spec);
  if (formula == NULL) {
    lhme_engine_destroy(engine);
    return 2;
  }

  const lhme_evaluation_request request = {
      10, LHME_MODE_EXACT, UINT64_MAX, NULL};
  lhme_result* result = lhme_evaluate(engine, formula, &request);
  const int failed = result == NULL ||
                     lhme_result_status(result) != LHME_STATUS_CONFIRMED ||
                     strcmp(lhme_result_exact_decimal(result), "102300") != 0;
  lhme_result_destroy(result);
  lhme_formula_destroy(formula);
  lhme_engine_destroy(engine);
  return failed;
}
