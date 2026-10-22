#ifndef AXIOM_LHME_C_API_H
#define AXIOM_LHME_C_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct axiom_lhme_engine axiom_lhme_engine;
typedef struct axiom_lhme_formula axiom_lhme_formula;
typedef struct axiom_lhme_result axiom_lhme_result;

typedef enum axiom_lhme_status {
  AXIOM_LHME_STATUS_CONFIRMED,
  AXIOM_LHME_STATUS_REJECTED,
  AXIOM_LHME_STATUS_BUDGET_EXCEEDED,
  AXIOM_LHME_STATUS_CANCELLED,
  AXIOM_LHME_STATUS_INVALID_ARGUMENT,
  AXIOM_LHME_STATUS_UNSUPPORTED_MODE,
  AXIOM_LHME_STATUS_INTERNAL_ERROR
} axiom_lhme_status;

typedef enum axiom_lhme_mode {
  AXIOM_LHME_MODE_SYMBOLIC,
  AXIOM_LHME_MODE_METRIC,
  AXIOM_LHME_MODE_MODULAR,
  AXIOM_LHME_MODE_EXACT
} axiom_lhme_mode;

typedef struct axiom_lhme_engine_config {
  size_t cache_max_entries;
  size_t cache_max_bytes;
  uint32_t worker_count;
  uint64_t tile_size;
} axiom_lhme_engine_config;

typedef struct axiom_lhme_power_offset_spec {
  const char* base_decimal;
  const char* scale_decimal;
  uint32_t version;
} axiom_lhme_power_offset_spec;

typedef struct axiom_lhme_evaluation_request {
  int64_t index;
  axiom_lhme_mode mode;
  uint64_t max_bit_length;
  const char* modulus_decimal;
} axiom_lhme_evaluation_request;

axiom_lhme_engine* axiom_lhme_engine_create(const axiom_lhme_engine_config* config);
void axiom_lhme_engine_destroy(axiom_lhme_engine* engine);
axiom_lhme_formula* axiom_lhme_formula_register(axiom_lhme_engine* engine,
                                    const axiom_lhme_power_offset_spec* spec);
void axiom_lhme_formula_destroy(axiom_lhme_formula* formula);
axiom_lhme_result* axiom_lhme_evaluate(axiom_lhme_engine* engine, const axiom_lhme_formula* formula,
                           const axiom_lhme_evaluation_request* request);
axiom_lhme_status axiom_lhme_result_status(const axiom_lhme_result* result);
const char* axiom_lhme_result_exact_decimal(const axiom_lhme_result* result);
const char* axiom_lhme_result_inspect(const axiom_lhme_result* result);
void axiom_lhme_result_destroy(axiom_lhme_result* result);
void axiom_lhme_cancel(axiom_lhme_engine* engine);

#ifdef __cplusplus
}
#endif

#endif
