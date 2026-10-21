#include "bruce/lhme/fibonacci_kernel.hpp"

#include "bruce/lhme/trace.hpp"

#include <limits>
#include <optional>

namespace bruce::lhme {
namespace {

struct FibonacciPair {
  BigInt current;
  BigInt next;
};

std::uint64_t saturating_add(std::uint64_t left, std::uint64_t right) {
  return left > std::numeric_limits<std::uint64_t>::max() - right
             ? std::numeric_limits<std::uint64_t>::max()
             : left + right;
}

std::uint64_t saturating_multiply(std::uint64_t left, std::uint64_t right) {
  if (left == 0 || right == 0) return 0;
  return left > std::numeric_limits<std::uint64_t>::max() / right
             ? std::numeric_limits<std::uint64_t>::max()
             : left * right;
}

std::uint64_t bit_length(std::uint64_t value) {
  std::uint64_t bits = 0;
  while (value > 0) {
    ++bits;
    value >>= 1U;
  }
  return bits;
}

std::uint64_t metric_upper_bound(std::uint64_t index) {
  const auto whole_tens = index / 10U;
  const auto remainder = index % 10U;
  return saturating_add(saturating_multiply(whole_tens, 7U),
                        (remainder * 7U + 9U) / 10U);
}

std::uint64_t bits_to_bytes(std::uint64_t bits) {
  return bits / 8U + (bits % 8U == 0U ? 0U : 1U);
}

bool cancelled(const std::shared_ptr<const std::atomic_bool>& signal) {
  return signal != nullptr && signal->load();
}

EvaluationResult traced(Status status, EvaluationMode mode, GateScope scope,
                        const char* reason) {
  EvaluationResult result;
  result.status = status;
  result.mode = mode;
  result.trace = make_trace({{.scope = scope, .status = status, .reason = reason}});
  return result;
}

EvaluationResult invalid_request(EvaluationMode mode) {
  return traced(Status::InvalidArgument, mode, GateScope::Branch,
                "invalid fibonacci request");
}

EvaluationResult cancelled_result(EvaluationMode mode) {
  return traced(Status::Cancelled, mode, GateScope::Run,
                "request cancellation requested");
}

EvaluationResult budget_exceeded(EvaluationMode mode, std::uint64_t metric) {
  auto result = traced(Status::BudgetExceeded, mode, GateScope::Publish,
                       "fibonacci exact work exceeded resource budget");
  result.metric_bit_length = metric;
  return result;
}

std::optional<FibonacciPair> fibonacci_pair(
    std::uint64_t index, const std::optional<BigInt>& modulus,
    const std::shared_ptr<const std::atomic_bool>& cancellation_signal) {
  FibonacciPair pair{.current = 0, .next = modulus.has_value() ? BigInt{1} % *modulus
                                                                 : BigInt{1}};
  if (index == 0) return pair;

  std::uint64_t mask = 1;
  while (mask <= index / 2U) {
    mask <<= 1U;
  }
  while (mask > 0) {
    if (cancelled(cancellation_signal)) return std::nullopt;

    BigInt doubled_current;
    BigInt doubled_next;
    if (modulus.has_value()) {
      const auto& mod = *modulus;
      auto difference = (BigInt{2} * pair.next - pair.current) % mod;
      if (difference < 0) difference += mod;
      doubled_current = (pair.current * difference) % mod;
      doubled_next = (pair.current * pair.current + pair.next * pair.next) % mod;
    } else {
      doubled_current = pair.current * (BigInt{2} * pair.next - pair.current);
      doubled_next = pair.current * pair.current + pair.next * pair.next;
    }

    if ((index & mask) == 0U) {
      pair.current = std::move(doubled_current);
      pair.next = std::move(doubled_next);
    } else if (modulus.has_value()) {
      pair.current = std::move(doubled_next);
      pair.next = (doubled_current + pair.current) % *modulus;
    } else {
      pair.current = std::move(doubled_next);
      pair.next = doubled_current + pair.current;
    }
    mask >>= 1U;
  }
  return pair;
}

}  // namespace

TechniqueIdentity FibonacciKernel::identity() const {
  return {.id = "bruce.fibonacci", .version = 1};
}

EvaluationResult FibonacciKernel::evaluate(const TechniqueRequest& request) const {
  if (request.identity != identity() ||
      request.parameters != kFibonacciFastDoublingParameters ||
      request.address.index < 0) {
    return invalid_request(request.mode);
  }
  if (cancelled(request.cancellation_signal)) {
    return cancelled_result(request.mode);
  }

  const auto index = static_cast<std::uint64_t>(request.address.index);
  const auto metric = metric_upper_bound(index);
  switch (request.mode) {
    case EvaluationMode::Symbolic: {
      EvaluationResult result;
      result.status = Status::Confirmed;
      result.mode = request.mode;
      result.symbolic_expression = "F(" + std::to_string(index) + ")";
      result.trace = make_trace({});
      return result;
    }
    case EvaluationMode::Metric: {
      EvaluationResult result;
      result.status = Status::Confirmed;
      result.mode = request.mode;
      result.metric_bit_length = metric;
      result.trace = make_trace({});
      return result;
    }
    case EvaluationMode::Modular:
      if (!request.modulus.has_value() || *request.modulus <= 0) {
        return invalid_request(request.mode);
      }
      if (const auto pair = fibonacci_pair(index, request.modulus,
                                           request.cancellation_signal);
          pair.has_value()) {
        EvaluationResult result;
        result.status = Status::Confirmed;
        result.mode = request.mode;
        result.integer = pair->current;
        result.trace = make_trace({});
        return result;
      }
      return cancelled_result(request.mode);
    case EvaluationMode::Exact: {
      const auto operations = saturating_multiply(bit_length(index), 2U);
      const auto temporary_bytes =
          saturating_multiply(bits_to_bytes(metric), 8U);
      if (metric > request.budget.max_bit_length ||
          operations > request.budget.max_operations ||
          temporary_bytes > request.budget.max_temporary_bytes) {
        return budget_exceeded(request.mode, metric);
      }
      if (const auto pair = fibonacci_pair(index, std::nullopt,
                                           request.cancellation_signal);
          pair.has_value()) {
        EvaluationResult result;
        result.status = Status::Confirmed;
        result.mode = request.mode;
        result.integer = pair->current;
        result.trace = make_trace({});
        return result;
      }
      return cancelled_result(request.mode);
    }
  }
  return traced(Status::InternalError, request.mode, GateScope::Branch,
                "unknown fibonacci evaluation mode");
}

}  // namespace bruce::lhme
