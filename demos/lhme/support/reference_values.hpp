#pragma once

#include <cstdint>

#include "bruce/lhme/formula.hpp"

namespace bruce::lhme::demo {

inline FormulaSpec reference_spec() { return make_power_offset_spec(2, 100, 1); }
inline BigInt exact_at_10() { return 102300; }
inline BigInt exact_at_20() { return 104857500; }
inline BigInt modulo_17_at_10() { return 11; }
inline constexpr std::int64_t large_metric_index() { return 1'000'000; }
inline constexpr std::uint64_t large_metric_bit_length() { return 1'000'007; }
inline constexpr std::uint64_t large_metric_minimum_bytes() { return 125001; }

}  // namespace bruce::lhme::demo
