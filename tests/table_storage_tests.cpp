#include "test.hpp"

#include "kevin_sparse_table.hpp"
#include "table_storage.hpp"

#include <cstdint>

namespace {

using bruce::lhme::benchmark::table::DenseValueTable;
using bruce::lhme::benchmark::table::FormulaRecipeTable;
using bruce::lhme::benchmark::table::HashValueTable;
using bruce::lhme::benchmark::table::FormulaOverrideTable;
using bruce::lhme::benchmark::table::SortedPairTable;
using bruce::lhme::benchmark::table::synthetic_value_at;

}  // namespace

TEST(materialized_table_variants_recover_the_same_value) {
  constexpr std::uint64_t record_count = 32;
  constexpr std::uint64_t address = 17;
  const auto expected = synthetic_value_at(address);

  const auto dense = DenseValueTable::make(record_count);
  const auto pairs = SortedPairTable::make(record_count);
  const auto hash = HashValueTable::make(record_count);

  CHECK(dense.value_at(address) == expected);
  CHECK(pairs.value_at(address) == expected);
  CHECK(hash.value_at(address) == expected);
  CHECK(dense.metrics().record_count == record_count);
  CHECK(pairs.metrics().record_count == record_count);
  CHECK(hash.metrics().record_count == record_count);
  CHECK(dense.metrics().value_payload_bytes == record_count * 8);
  CHECK(pairs.metrics().value_payload_bytes == record_count * 8);
  CHECK(pairs.metrics().address_metadata_bytes == record_count * 8);
  CHECK(hash.metrics().value_payload_bytes == record_count * 8);
  CHECK(hash.metrics().address_metadata_bytes == record_count * 8);
}

TEST(formula_recipe_keeps_a_million_addresses_without_a_million_cells) {
  constexpr std::uint64_t record_count = 1'000'000;
  const auto recipe = FormulaRecipeTable::make(record_count);

  CHECK(recipe.contains(0));
  CHECK(recipe.contains(record_count - 1));
  CHECK(!recipe.contains(record_count));
  CHECK(recipe.symbolic_value_at(42).value_or("") == "100*(2^42-1)");
  CHECK(!recipe.symbolic_value_at(record_count).has_value());
  CHECK(recipe.specification().base == 2);
  CHECK(recipe.specification().scale == 100);
  CHECK(recipe.specification().version == 1);
  CHECK(recipe.metrics().record_count == record_count);
  CHECK(recipe.metrics().container_bytes < 1'024);
}

TEST(formula_override_table_keeps_formula_addresses_lazy_and_applies_exceptions) {
  constexpr std::uint64_t record_count = 1'000'000;
  auto table = FormulaOverrideTable::make(record_count, 10'000);

  CHECK(table.contains(0));
  CHECK(table.contains(record_count - 1));
  CHECK(!table.contains(record_count));
  CHECK(table.override_count() == 10'000);
  CHECK(table.source_at(0) == FormulaOverrideTable::ValueSource::Override);
  CHECK(table.source_at(record_count - 1) ==
        FormulaOverrideTable::ValueSource::Formula);
}
