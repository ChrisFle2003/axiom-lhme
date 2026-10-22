#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <memory>
#include <stdexcept>
#include <string>

#include <boost/multiprecision/cpp_int.hpp>

namespace axiom::lhme {

struct TraceRecord;

using BigInt = boost::multiprecision::cpp_int;

enum class EvaluationMode {
  Symbolic,
  Metric,
  Modular,
  Exact,
};

enum class Status {
  Confirmed,
  Rejected,
  BudgetExceeded,
  Cancelled,
  InvalidArgument,
  UnsupportedMode,
  InternalError,
};

struct Budget {
  std::uint64_t max_bit_length = std::numeric_limits<std::uint64_t>::max();
  std::uint64_t max_operations = std::numeric_limits<std::uint64_t>::max();
  std::uint64_t max_temporary_bytes = std::numeric_limits<std::uint64_t>::max();
  std::uint64_t max_trace_bytes = std::numeric_limits<std::uint64_t>::max();
  std::uint64_t max_candidate_count = std::numeric_limits<std::uint64_t>::max();

  [[nodiscard]] static Budget unlimited() { return {}; }
};

struct FormulaAddress {
  std::int64_t index = 0;
};

struct EvaluationResult {
  Status status = Status::InternalError;
  EvaluationMode mode = EvaluationMode::Symbolic;
  std::optional<BigInt> integer;
  std::optional<std::uint64_t> metric_bit_length;
  std::optional<std::string> symbolic_expression;
  std::shared_ptr<const TraceRecord> trace;

  [[nodiscard]] const BigInt& exact_integer() const {
    if (!integer.has_value()) {
      throw std::logic_error("result does not contain an exact integer");
    }
    return *integer;
  }
};

}  // namespace axiom::lhme
