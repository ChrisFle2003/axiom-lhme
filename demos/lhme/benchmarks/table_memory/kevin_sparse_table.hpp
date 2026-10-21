#pragma once

#include "bruce/lhme/formula_override_store.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace bruce::lhme::benchmark::table {

// Kept in the former fixture path so downstream benchmark include paths do not
// change. The fixture deliberately has no GridAddress or KEVIN dependency.
class FormulaOverrideTable {
 public:
  enum class ValueSource { Formula, Override };

  [[nodiscard]] static FormulaOverrideTable make(std::uint64_t record_count,
                                                  std::uint64_t override_count) {
    return FormulaOverrideTable(record_count, override_count);
  }

  [[nodiscard]] bool contains(std::uint64_t address) const noexcept {
    return address < record_count_;
  }

  [[nodiscard]] std::uint64_t record_count() const noexcept { return record_count_; }
  [[nodiscard]] std::uint64_t override_count() const noexcept { return override_count_; }

  [[nodiscard]] ValueSource source_at(std::uint64_t address) {
    if (!contains(address)) {
      throw std::out_of_range("address is not present in formula table");
    }
    const auto value = store_.evaluate({.binding = binding_,
                                        .address = {.index = static_cast<std::int64_t>(address)},
                                        .mode = EvaluationMode::Metric,
                                        .budget = Budget::unlimited()});
    if (value.result.status != Status::Confirmed) {
      throw std::runtime_error("formula metric lookup failed");
    }
    return value.source == FormulaValueSource::Override ? ValueSource::Override
                                                         : ValueSource::Formula;
  }

 private:
  FormulaOverrideTable(std::uint64_t record_count, std::uint64_t override_count)
      : engine_(Engine::create({})), store_(engine_), record_count_(record_count),
        override_count_(override_count) {
    if (record_count_ == 0 || override_count_ > record_count_ ||
        record_count_ > static_cast<std::uint64_t>(
                            std::numeric_limits<std::int64_t>::max())) {
      throw std::invalid_argument("formula table dimensions are invalid");
    }
    binding_ = engine_.register_formula(make_power_offset_spec(2, 100, 1));
    for (std::uint64_t ordinal = 0; ordinal < override_count_; ++ordinal) {
      const auto address = ordinal * record_count_ / override_count_;
      if (store_.set_override(binding_,
                              {.index = static_cast<std::int64_t>(address)},
                              BigInt{static_cast<std::int64_t>(ordinal % 97U + 1U)}) !=
          Status::Confirmed) {
        throw std::runtime_error("formula override insertion failed");
      }
    }
  }

  Engine engine_;
  FormulaBinding binding_;
  FormulaOverrideStore store_;
  std::uint64_t record_count_ = 0;
  std::uint64_t override_count_ = 0;
};

}  // namespace bruce::lhme::benchmark::table
