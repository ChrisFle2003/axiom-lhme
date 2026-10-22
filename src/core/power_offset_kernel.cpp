#include "axiom/lhme/power_offset_kernel.hpp"

#include <charconv>
#include <cctype>

namespace axiom::lhme {
namespace {

[[nodiscard]] std::optional<std::string_view> parse_decimal_field(
    std::string_view input, std::size_t& cursor, std::string_view name,
    bool requires_separator) {
  const auto prefix = std::string{name} + "=";
  if (input.substr(cursor, prefix.size()) != prefix) return std::nullopt;
  cursor += prefix.size();

  const auto length_begin = cursor;
  while (cursor < input.size() &&
         std::isdigit(static_cast<unsigned char>(input[cursor])) != 0) {
    ++cursor;
  }
  if (length_begin == cursor || cursor == input.size() || input[cursor] != ':') {
    return std::nullopt;
  }

  std::size_t field_length = 0;
  const auto length_text = input.substr(length_begin, cursor - length_begin);
  const auto [end, error] = std::from_chars(length_text.data(),
                                             length_text.data() + length_text.size(),
                                             field_length);
  if (error != std::errc{} || end != length_text.data() + length_text.size()) {
    return std::nullopt;
  }
  if (length_text != std::to_string(field_length)) return std::nullopt;
  ++cursor;
  if (field_length == 0 || field_length > input.size() - cursor) return std::nullopt;

  const auto field = input.substr(cursor, field_length);
  cursor += field_length;
  if (field.front() == '0' && field.size() > 1) return std::nullopt;
  for (const auto character : field) {
    if (std::isdigit(static_cast<unsigned char>(character)) == 0) return std::nullopt;
  }
  if (requires_separator) {
    if (cursor == input.size() || input[cursor] != ';') return std::nullopt;
    ++cursor;
  }
  return field;
}

[[nodiscard]] EvaluationResult invalid_request(EvaluationMode mode) {
  EvaluationResult result;
  result.status = Status::InvalidArgument;
  result.mode = mode;
  return result;
}

}  // namespace

std::optional<PowerOffsetParameters> parse_power_offset_parameters(
    std::string_view parameters) {
  std::size_t cursor = 0;
  const auto base_text = parse_decimal_field(parameters, cursor, "base", true);
  const auto scale_text = parse_decimal_field(parameters, cursor, "scale", false);
  if (!base_text.has_value() || !scale_text.has_value() || cursor != parameters.size()) {
    return std::nullopt;
  }

  const BigInt base{std::string{*base_text}};
  const BigInt scale{std::string{*scale_text}};
  if (base <= 0 || scale < 0 ||
      encode_power_offset_parameters(base, scale) != parameters) {
    return std::nullopt;
  }
  return PowerOffsetParameters{.base = base, .scale = scale};
}

TechniqueIdentity PowerOffsetKernel::identity() const {
  return {.id = "axiom.power_offset", .version = 1};
}

EvaluationResult PowerOffsetKernel::evaluate(const TechniqueRequest& request) const {
  if (request.identity != identity() || request.address.index < 0) {
    return invalid_request(request.mode);
  }
  const auto parameters = parse_power_offset_parameters(request.parameters);
  if (!parameters.has_value()) return invalid_request(request.mode);

  return axiom::lhme::evaluate({
      .spec = make_power_offset_spec(parameters->base, parameters->scale,
                                     request.identity.version),
      .address = request.address,
      .mode = request.mode,
      .budget = request.budget,
      .modulus = request.modulus,
      .cancellation_signal = request.cancellation_signal});
}

}  // namespace axiom::lhme
