#include "test.hpp"

#include "axiom/lhme/cache.hpp"
#include "axiom/lhme/technique.hpp"

namespace {

axiom::lhme::FormulaSpec spec(std::uint32_t version = 1) {
  return axiom::lhme::make_power_offset_spec(2, 100, version);
}

axiom::lhme::EvaluationResult confirmed(std::int64_t index) {
  return axiom::lhme::evaluate({.spec = spec(),
                                .address = {.index = index},
                                .mode = axiom::lhme::EvaluationMode::Exact,
                                .budget = axiom::lhme::Budget::unlimited()});
}

}  // namespace

TEST(cache_round_trip_preserves_confirmed_result) {
  axiom::lhme::ResultCache cache({.max_entries = 4, .max_bytes = 1024});
  const auto key = axiom::lhme::CacheKey::exact(spec(), 10);
  const auto original = confirmed(10);
  cache.admit(key, original);
  const auto cached = cache.lookup(key);

  CHECK(cached.has_value());
  CHECK(cached->exact_integer() == original.exact_integer());
  CHECK(cached->status == original.status);
}

TEST(mode_or_version_mismatch_is_a_cache_miss) {
  axiom::lhme::ResultCache cache({.max_entries = 4, .max_bytes = 1024});
  cache.admit(axiom::lhme::CacheKey::exact(spec(), 10), confirmed(10));

  CHECK(!cache.lookup(axiom::lhme::CacheKey::metric(spec(), 10)).has_value());
  CHECK(!cache.lookup(axiom::lhme::CacheKey::exact(spec(2), 10)).has_value());
}

TEST(cache_never_exceeds_configured_byte_limit) {
  axiom::lhme::ResultCache cache({.max_entries = 8, .max_bytes = 1});
  cache.admit(axiom::lhme::CacheKey::exact(spec(), 10), confirmed(10));
  CHECK(cache.statistics().resident_bytes <= 1);
}

TEST(cache_byte_budget_counts_technique_parameter_bytes) {
  axiom::lhme::ResultCache cache({.max_entries = 4, .max_bytes = 512});
  const auto metric = axiom::lhme::evaluate(
      {.spec = spec(),
       .address = {.index = 10},
       .mode = axiom::lhme::EvaluationMode::Metric,
       .budget = axiom::lhme::Budget::unlimited()});

  cache.admit(
      axiom::lhme::CacheKey::technique(
          {.id = "axiom.test", .version = 1}, std::string(1024, 'p'), 1,
          axiom::lhme::EvaluationMode::Metric),
      metric);

  CHECK(cache.statistics().resident_entries == 0);
  CHECK(cache.statistics().resident_bytes == 0);
}

TEST(cache_replacing_an_entry_keeps_resident_bytes_accurate) {
  axiom::lhme::ResultCache cache({.max_entries = 4, .max_bytes = 4096});
  const auto key = axiom::lhme::CacheKey::exact(spec(), 5);

  cache.admit(key, confirmed(5));
  const auto first_bytes = cache.statistics().resident_bytes;
  cache.admit(key, confirmed(5));

  CHECK(cache.statistics().resident_entries == 1);
  CHECK(first_bytes > 0);
  CHECK(cache.statistics().resident_bytes == first_bytes);
}

TEST(cache_with_a_zero_byte_budget_rejects_metric_entries_too) {
  axiom::lhme::ResultCache cache({.max_entries = 4, .max_bytes = 0});
  const auto value = axiom::lhme::evaluate({.spec = spec(),
                                             .address = {.index = 10},
                                             .mode = axiom::lhme::EvaluationMode::Metric,
                                             .budget = axiom::lhme::Budget::unlimited()});

  cache.admit(axiom::lhme::CacheKey::metric(spec(), 10), value);

  CHECK(cache.statistics().resident_entries == 0);
}

TEST(technique_cache_key_separates_length_ambiguous_parameters) {
  const auto first = axiom::lhme::CacheKey::technique(
      {.id = "example", .version = 1}, "base=1:1;scale=2:23", 10,
      axiom::lhme::EvaluationMode::Exact);
  const auto second = axiom::lhme::CacheKey::technique(
      {.id = "example", .version = 1}, "base=2:12;scale=1:3", 10,
      axiom::lhme::EvaluationMode::Exact);

  CHECK(first != second);
}
