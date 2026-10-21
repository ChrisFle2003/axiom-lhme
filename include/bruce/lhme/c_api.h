#ifndef BRUCE_LHME_C_API_H
#define BRUCE_LHME_C_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lhme_engine lhme_engine;
typedef struct lhme_formula lhme_formula;
typedef struct lhme_result lhme_result;

typedef enum lhme_status {
  LHME_STATUS_CONFIRMED,
  LHME_STATUS_REJECTED,
  LHME_STATUS_BUDGET_EXCEEDED,
  LHME_STATUS_CANCELLED,
  LHME_STATUS_INVALID_ARGUMENT,
  LHME_STATUS_UNSUPPORTED_MODE,
  LHME_STATUS_INTERNAL_ERROR
} lhme_status;

typedef enum lhme_mode {
  LHME_MODE_SYMBOLIC,
  LHME_MODE_METRIC,
  LHME_MODE_MODULAR,
  LHME_MODE_EXACT
} lhme_mode;

typedef struct lhme_engine_config {
  size_t cache_max_entries;
  size_t cache_max_bytes;
  uint32_t worker_count;
  uint64_t tile_size;
} lhme_engine_config;

typedef struct lhme_power_offset_spec {
  const char* base_decimal;
  const char* scale_decimal;
  uint32_t version;
} lhme_power_offset_spec;

typedef struct lhme_evaluation_request {
  int64_t index;
  lhme_mode mode;
  uint64_t max_bit_length;
  const char* modulus_decimal;
} lhme_evaluation_request;

lhme_engine* lhme_engine_create(const lhme_engine_config* config);
void lhme_engine_destroy(lhme_engine* engine);
lhme_formula* lhme_formula_register(lhme_engine* engine,
                                    const lhme_power_offset_spec* spec);
void lhme_formula_destroy(lhme_formula* formula);
lhme_result* lhme_evaluate(lhme_engine* engine, const lhme_formula* formula,
                           const lhme_evaluation_request* request);
lhme_status lhme_result_status(const lhme_result* result);
const char* lhme_result_exact_decimal(const lhme_result* result);
const char* lhme_result_inspect(const lhme_result* result);
void lhme_result_destroy(lhme_result* result);
void lhme_cancel(lhme_engine* engine);

#ifdef __cplusplus
}
#endif

#endif
