#include "axiom/lhme/formula_override_snapshot.hpp"

#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>

namespace axiom::lhme {
namespace {

constexpr char kMagic[] = "AXIOM-FORMULA-OVERRIDE-1\n";
constexpr std::size_t kMaximumRecords = 100'000;
constexpr std::size_t kMaximumStringBytes = 1'000'000;

std::uint64_t checksum(std::string_view bytes) {
  std::uint64_t hash = 14695981039346656037ULL;
  for (const auto byte : bytes) {
    hash ^= static_cast<unsigned char>(byte);
    hash *= 1099511628211ULL;
  }
  return hash;
}

bool write_string(std::ostream& output, const std::string& value) {
  output << value.size() << ':' << value << '\n';
  return static_cast<bool>(output);
}

bool read_string(std::istream& input, std::string& value) {
  std::size_t size = 0;
  char colon = 0;
  if (!(input >> size) || !(input >> colon) || colon != ':' ||
      size > kMaximumStringBytes) {
    return false;
  }
  value.resize(size);
  input.read(value.data(), static_cast<std::streamsize>(size));
  char newline = 0;
  input.get(newline);
  return static_cast<bool>(input) && newline == '\n';
}

bool canonical_integer(const std::string& text) {
  if (text.empty()) return false;
  const std::size_t first_digit = text.front() == '-' ? 1 : 0;
  if (first_digit == text.size()) return false;
  if (text[first_digit] == '0' && text.size() - first_digit > 1) return false;
  for (std::size_t index = first_digit; index < text.size(); ++index) {
    if (text[index] < '0' || text[index] > '9') return false;
  }
  return !(text.size() > 2 && text.front() == '-' && text[1] == '0');
}

bool write_record(std::ostream& output, const FormulaOverrideRecord& record) {
  if (const auto* formula = std::get_if<FormulaSpec>(&record.binding)) {
    if (formula->operation != FormulaOperation::PowerOffset) return false;
    output << "F " << formula->version << ' '
           << (formula->nonnegative_index ? 1 : 0) << ' ' << record.address.index
           << '\n';
    return write_string(output, formula->base.convert_to<std::string>()) &&
           write_string(output, formula->scale.convert_to<std::string>()) &&
           write_string(output, record.value.convert_to<std::string>());
  }

  const auto& technique = std::get<TechniqueBinding>(record.binding);
  output << "T " << technique.identity.version << ' ' << record.address.index << ' '
         << technique.default_budget.max_bit_length << ' '
         << technique.default_budget.max_operations << ' '
         << technique.default_budget.max_temporary_bytes << ' '
         << technique.default_budget.max_trace_bytes << ' '
         << technique.default_budget.max_candidate_count << '\n';
  return write_string(output, technique.identity.id) &&
         write_string(output, technique.canonical_parameters) &&
         write_string(output, record.value.convert_to<std::string>());
}

bool read_big_int(std::istream& input, BigInt& value) {
  std::string text;
  if (!read_string(input, text) || !canonical_integer(text)) return false;
  try {
    value = BigInt{text};
  } catch (...) {
    return false;
  }
  return true;
}

bool read_record(std::istream& input, FormulaOverrideRecord& record) {
  char kind = 0;
  if (!(input >> kind)) return false;
  if (kind == 'F') {
    std::uint32_t version = 0;
    unsigned nonnegative = 0;
    FormulaAddress address;
    if (!(input >> version >> nonnegative >> address.index) || nonnegative > 1) {
      return false;
    }
    input.ignore(1);
    std::string base;
    std::string scale;
    BigInt value;
    if (!read_string(input, base) || !read_string(input, scale) ||
        !canonical_integer(base) || !canonical_integer(scale) ||
        !read_big_int(input, value)) {
      return false;
    }
    try {
      record.binding = make_power_offset_spec(BigInt{base}, BigInt{scale}, version);
    } catch (...) {
      return false;
    }
    auto& formula = std::get<FormulaSpec>(record.binding);
    formula.nonnegative_index = nonnegative == 1;
    record.address = address;
    record.value = std::move(value);
    return true;
  }
  if (kind == 'T') {
    TechniqueBinding binding;
    FormulaAddress address;
    if (!(input >> binding.identity.version >> address.index >>
          binding.default_budget.max_bit_length >>
          binding.default_budget.max_operations >>
          binding.default_budget.max_temporary_bytes >>
          binding.default_budget.max_trace_bytes >>
          binding.default_budget.max_candidate_count)) {
      return false;
    }
    input.ignore(1);
    BigInt value;
    if (!read_string(input, binding.identity.id) ||
        !read_string(input, binding.canonical_parameters) ||
        !read_big_int(input, value)) {
      return false;
    }
    record.binding = std::move(binding);
    record.address = address;
    record.value = std::move(value);
    return true;
  }
  return false;
}

}  // namespace

FormulaOverrideSnapshotStatus save_formula_override_snapshot(
    const FormulaOverrideStore& store, const std::filesystem::path& path) {
  const auto temporary = path.string() + ".tmp";
  std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
  if (!output) return FormulaOverrideSnapshotStatus::IoError;
  const auto snapshot = store.snapshot();
  output << kMagic << snapshot.overrides.size() << '\n';
  for (const auto& record : snapshot.overrides) {
    if (!write_record(output, record)) return FormulaOverrideSnapshotStatus::IoError;
  }
  output.close();
  if (!output) return FormulaOverrideSnapshotStatus::IoError;

  std::ifstream payload_input(temporary, std::ios::binary);
  const std::string payload{std::istreambuf_iterator<char>(payload_input), {}};
  if (!payload_input.good() && !payload_input.eof()) {
    return FormulaOverrideSnapshotStatus::IoError;
  }
  std::ofstream checksum_output(temporary, std::ios::binary | std::ios::app);
  checksum_output << "CHECKSUM " << checksum(payload) << '\n';
  checksum_output.close();
  if (!checksum_output) return FormulaOverrideSnapshotStatus::IoError;

  std::error_code error;
  std::filesystem::rename(temporary, path, error);
  if (error) {
    std::filesystem::remove(temporary);
    return FormulaOverrideSnapshotStatus::IoError;
  }
  return FormulaOverrideSnapshotStatus::Confirmed;
}

FormulaOverrideSnapshotStatus load_formula_override_snapshot(
    FormulaOverrideStore& store, const std::filesystem::path& path) {
  std::ifstream source(path, std::ios::binary);
  if (!source) return FormulaOverrideSnapshotStatus::IoError;
  const std::string file{std::istreambuf_iterator<char>(source), {}};
  const auto marker = file.rfind("CHECKSUM ");
  if (marker == std::string::npos || marker == 0 || file[marker - 1] != '\n') {
    return FormulaOverrideSnapshotStatus::InvalidSnapshot;
  }
  std::uint64_t stored_checksum = 0;
  std::istringstream checksum_input(file.substr(marker + 9));
  std::string trailing_checksum_data;
  if (!(checksum_input >> stored_checksum) ||
      (checksum_input >> trailing_checksum_data) ||
      checksum(file.substr(0, marker)) != stored_checksum) {
    return FormulaOverrideSnapshotStatus::InvalidSnapshot;
  }

  std::istringstream input(file.substr(0, marker));
  std::string magic(sizeof(kMagic) - 1, '\0');
  input.read(magic.data(), static_cast<std::streamsize>(magic.size()));
  if (magic != kMagic) return FormulaOverrideSnapshotStatus::InvalidSnapshot;
  std::size_t count = 0;
  if (!(input >> count) || count > kMaximumRecords) {
    return FormulaOverrideSnapshotStatus::InvalidSnapshot;
  }
  input.ignore(1);

  FormulaOverrideSnapshot snapshot;
  snapshot.overrides.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    FormulaOverrideRecord record;
    if (!read_record(input, record)) {
      return FormulaOverrideSnapshotStatus::InvalidSnapshot;
    }
    snapshot.overrides.push_back(std::move(record));
  }
  if (input.peek() != std::char_traits<char>::eof()) {
    return FormulaOverrideSnapshotStatus::InvalidSnapshot;
  }
  return store.replace_snapshot(snapshot) == FormulaOverrideSnapshotStatus::Confirmed
             ? FormulaOverrideSnapshotStatus::Confirmed
             : FormulaOverrideSnapshotStatus::InvalidSnapshot;
}

}  // namespace axiom::lhme
