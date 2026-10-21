#include "bruce/lhme/formula.hpp"

#include "bruce/lhme/trace.hpp"

#include <array>
#include <cstdint>
#include <string>

namespace bruce::lhme {
namespace {

constexpr std::uint64_t kFnvOffset = 14695981039346656037ULL;
constexpr std::uint64_t kFnvPrime = 1099511628211ULL;

void mix_byte(std::uint64_t& state, std::byte value) {
  state ^= std::to_integer<std::uint8_t>(value);
  state *= kFnvPrime;
}

void mix_text(std::uint64_t& state, const std::string& value) {
  for (const auto character : value) {
    mix_byte(state, static_cast<std::byte>(static_cast<unsigned char>(character)));
  }
}

void mix_u64(std::uint64_t& state, std::uint64_t value) {
  for (std::size_t byte = 0; byte < 8; ++byte) {
    mix_byte(state, static_cast<std::byte>((value >> (byte * 8)) & 0xffU));
  }
}

void mix_field(std::uint64_t& state, const std::string& value) {
  mix_u64(state, value.size());
  mix_text(state, value);
}

std::string identity_from_parts(FormulaOperation operation, const BigInt& base,
                                const BigInt& scale, bool nonnegative_index,
                                std::uint32_t version) {
  const auto base_text = base.convert_to<std::string>();
  const auto scale_text = scale.convert_to<std::string>();
  return std::to_string(static_cast<std::uint64_t>(operation)) + ":" +
         std::to_string(nonnegative_index ? 1U : 0U) + ":" +
         std::to_string(version) + ":" + std::to_string(base_text.size()) +
         ":" + base_text + ":" + std::to_string(scale_text.size()) + ":" +
         scale_text;
}

std::array<std::byte, 32> hash_spec(FormulaOperation operation,
                                    const BigInt& base, const BigInt& scale,
                                    bool nonnegative_index,
                                    std::uint32_t version) {
  std::array<std::byte, 32> hash{};
  for (std::uint64_t lane = 0; lane < 4; ++lane) {
    std::uint64_t state = kFnvOffset ^ lane;
    mix_field(state, identity_from_parts(operation, base, scale,
                                         nonnegative_index, version));
    for (std::size_t byte = 0; byte < 8; ++byte) {
      hash[lane * 8 + byte] =
          static_cast<std::byte>((state >> (byte * 8)) & 0xffU);
    }
  }
  return hash;
}

std::optional<BigInt> power(
    BigInt base, std::uint64_t exponent,
    const std::shared_ptr<const std::atomic_bool>& cancellation_signal = nullptr) {
  BigInt result = 1;
  while (exponent > 0) {
    if (cancellation_signal != nullptr && cancellation_signal->load()) {
      return std::nullopt;
    }
    if ((exponent & 1U) != 0U) {
      result *= base;
    }
    exponent >>= 1U;
    if (exponent > 0) {
      base *= base;
    }
  }
  return result;
}

std::uint64_t bit_length(const BigInt& value) {
  if (value == 0) {
    return 0;
  }
  return static_cast<std::uint64_t>(boost::multiprecision::msb(value)) + 1U;
}

bool is_valid_power_offset_request(const FormulaSpec& spec,
                                   const FormulaAddress& address) {
  return spec.operation == FormulaOperation::PowerOffset && spec.base > 0 &&
         spec.scale >= 0 && address.index >= 0;
}

std::uint64_t add_saturating(std::uint64_t left, std::uint64_t right) {
  return left > std::numeric_limits<std::uint64_t>::max() - right
             ? std::numeric_limits<std::uint64_t>::max()
             : left + right;
}

std::uint64_t power_two_offset_bit_length(const BigInt& scale,
                                          std::uint64_t exponent) {
  if (scale == 0 || exponent == 0) return 0;
  const auto scale_bits = bit_length(scale);
  if (exponent < scale_bits) {
    return bit_length((scale << exponent) - scale);
  }
  const BigInt scale_floor = BigInt(1) << (scale_bits - 1U);
  return scale == scale_floor ? add_saturating(scale_bits, exponent - 1U)
                              : add_saturating(scale_bits, exponent);
}

std::uint64_t estimated_bit_length(const FormulaSpec& spec,
                                   std::uint64_t exponent) {
  if (exponent == 0 || spec.scale == 0 || spec.base == 1) {
    return 0;
  }
  if (spec.base == 2) return power_two_offset_bit_length(spec.scale, exponent);
  const auto base_bits = bit_length(spec.base);
  const auto scale_bits = bit_length(spec.scale);
  if (base_bits == 0 ||
      exponent > std::numeric_limits<std::uint64_t>::max() / base_bits) {
    return std::numeric_limits<std::uint64_t>::max();
  }
  return add_saturating(scale_bits, base_bits * exponent);
}

std::uint64_t estimated_operations(std::uint64_t exponent) {
  return exponent == 0 ? 0U : add_saturating(bit_length(BigInt(exponent)),
                                               bit_length(BigInt(exponent)));
}

std::uint64_t bits_to_bytes(std::uint64_t bits) {
  return bits / 8U + (bits % 8U == 0 ? 0U : 1U);
}

std::optional<BigInt> power_mod(
    BigInt base, std::uint64_t exponent, const BigInt& modulus,
    const std::shared_ptr<const std::atomic_bool>& cancellation_signal = nullptr) {
  BigInt result = 1 % modulus;
  base %= modulus;
  while (exponent > 0) {
    if (cancellation_signal != nullptr && cancellation_signal->load()) {
      return std::nullopt;
    }
    if ((exponent & 1U) != 0U) {
      result = (result * base) % modulus;
    }
    exponent >>= 1U;
    if (exponent > 0) {
      base = (base * base) % modulus;
    }
  }
  return result;
}

}  // namespace

FormulaSpec make_power_offset_spec(BigInt base, BigInt scale,
                                   std::uint32_t version) {
  FormulaSpec spec;
  spec.base = std::move(base);
  spec.scale = std::move(scale);
  spec.version = version;
  spec.specification_hash = hash_spec(spec.operation, spec.base, spec.scale,
                                      spec.nonnegative_index, spec.version);
  return spec;
}

std::string canonical_formula_identity(const FormulaSpec& spec) {
  return identity_from_parts(spec.operation, spec.base, spec.scale,
                             spec.nonnegative_index, spec.version);
}

EvaluationResult evaluate_power_offset_impl(
    const FormulaSpec& spec, const FormulaAddress& address,
    EvaluationMode mode, const Budget& budget,
    const std::shared_ptr<const std::atomic_bool>& cancellation_signal) {
  EvaluationResult result;
  result.mode = mode;

  if (!is_valid_power_offset_request(spec, address)) {
    result.status = Status::InvalidArgument;
    return result;
  }

  const auto exponent = static_cast<std::uint64_t>(address.index);
  switch (mode) {
    case EvaluationMode::Symbolic:
      result.status = Status::Confirmed;
      result.symbolic_expression =
          spec.scale.convert_to<std::string>() + "*(" +
          spec.base.convert_to<std::string>() + "^" +
          std::to_string(address.index) + "-1)";
      return result;
    case EvaluationMode::Metric: {
      result.status = Status::Confirmed;
      result.metric_bit_length = estimated_bit_length(spec, exponent);
      return result;
    }
    case EvaluationMode::Modular:
      result.status = Status::UnsupportedMode;
      return result;
    case EvaluationMode::Exact:
      const auto estimated_bits = estimated_bit_length(spec, exponent);
      if (estimated_bits > budget.max_bit_length ||
          estimated_operations(exponent) > budget.max_operations ||
          bits_to_bytes(estimated_bits) > budget.max_temporary_bytes) {
        result.status = Status::BudgetExceeded;
        result.metric_bit_length = estimated_bits;
        return result;
      }
      result.status = Status::Confirmed;
      if (spec.base == 1 || spec.scale == 0) {
        result.integer = 0;
      } else {
        const auto powered = power(spec.base, exponent, cancellation_signal);
        if (!powered.has_value()) {
          result.status = Status::Cancelled;
          return result;
        }
        result.integer = spec.scale * (*powered - 1);
      }
      return result;
  }
  result.status = Status::InternalError;
  return result;
}

EvaluationResult evaluate_power_offset(const FormulaSpec& spec,
                                       const FormulaAddress& address,
                                       EvaluationMode mode,
                                       const Budget& budget) {
  return evaluate_power_offset_impl(spec, address, mode, budget, nullptr);
}

EvaluationResult evaluate(const EvaluationRequest& request) {
  if (request.budget.max_trace_bytes == 0) {
    return {.status = Status::BudgetExceeded, .mode = request.mode};
  }
  if (request.cancellation_requested ||
      (request.cancellation_signal != nullptr &&
       request.cancellation_signal->load())) {
    EvaluationResult result;
    result.status = Status::Cancelled;
    result.mode = request.mode;
    result.trace = make_trace({{.scope = GateScope::Run,
                                .status = result.status,
                                .reason = "request cancellation requested"}});
    return result;
  }
  if (request.mode == EvaluationMode::Modular) {
    EvaluationResult result;
    result.mode = request.mode;
    if (!is_valid_power_offset_request(request.spec, request.address) ||
        !request.modulus.has_value() || *request.modulus <= 0) {
      result.status = Status::InvalidArgument;
      result.trace = make_trace({{.scope = GateScope::Branch,
                                  .status = result.status,
                                  .reason = "invalid modular request"}});
      return result;
    }
    const auto exponent = static_cast<std::uint64_t>(request.address.index);
    const auto base_power = power_mod(request.spec.base, exponent,
                                      *request.modulus,
                                      request.cancellation_signal);
    if (!base_power.has_value()) {
      result.status = Status::Cancelled;
      result.trace = make_trace({{.scope = GateScope::Run,
                                  .status = result.status,
                                  .reason = "request cancellation requested"}});
      return result;
    }
    BigInt value = (request.spec.scale % *request.modulus) *
                   ((*base_power - 1) % *request.modulus);
    value %= *request.modulus;
    if (value < 0) {
      value += *request.modulus;
    }
    result.status = Status::Confirmed;
    result.integer = std::move(value);
    result.trace = make_trace({});
    return result;
  }

  EvaluationResult result = evaluate_power_offset_impl(
      request.spec, request.address, request.mode, request.budget,
      request.cancellation_signal);
  if (result.status == Status::InvalidArgument) {
    result.trace = make_trace({{.scope = GateScope::Branch,
                                .status = result.status,
                                .reason = "invalid formula address or spec"}});
  } else if (result.status == Status::BudgetExceeded) {
    result.trace = make_trace({{.scope = GateScope::Publish,
                                .status = result.status,
                                .reason = "exact expansion exceeded bit budget"}});
  } else if (result.status == Status::Cancelled) {
    result.trace = make_trace({{.scope = GateScope::Run,
                                .status = result.status,
                                .reason = "request cancellation requested"}});
  } else {
    result.trace = make_trace({});
  }
  return result;
}

}  // namespace bruce::lhme
