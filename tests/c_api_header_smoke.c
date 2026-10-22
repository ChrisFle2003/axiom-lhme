#include <stdint.h>
#include <string.h>

#include "axiom/lhme/axiom_lhme_c_api.h"

int main(void) {
  const axiom_lhme_engine_config config = {8, 4096, 1, 4};
  axiom_lhme_engine* engine = axiom_lhme_engine_create(&config);
  if (engine == NULL) return 1;

  const axiom_lhme_power_offset_spec spec = {"2", "100", 1};
  axiom_lhme_formula* formula = axiom_lhme_formula_register(engine, &spec);
  if (formula == NULL) {
    axiom_lhme_engine_destroy(engine);
    return 2;
  }

  const axiom_lhme_evaluation_request request = {
      10, AXIOM_LHME_MODE_EXACT, UINT64_MAX, NULL};
  axiom_lhme_result* result = axiom_lhme_evaluate(engine, formula, &request);
  const int failed = result == NULL ||
                     axiom_lhme_result_status(result) != AXIOM_LHME_STATUS_CONFIRMED ||
                     strcmp(axiom_lhme_result_exact_decimal(result), "102300") != 0;
  axiom_lhme_result_destroy(result);
  axiom_lhme_formula_destroy(formula);
  axiom_lhme_engine_destroy(engine);
  return failed;
}
