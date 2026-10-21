#pragma once

#include "bruce/lhme/formula.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace bruce::lhme::benchmark::table {

struct StorageMetrics {
  std::uint64_t record_count = 0;
  std::uint64_t value_payload_bytes = 0;
  std::uint64_t address_metadata_bytes = 0;
  std::uint64_t container_bytes = 0;
};

[[nodiscard]] inline constexpr std::uint64_t synthetic_value_at(
    std::uint64_t address) {
  auto value = address + 0x9e3779b97f4a7c15ULL;
  value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
  value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
  return value ^ (value >> 31U);
}

class DenseValueTable {
 public:
  [[nodiscard]] static DenseValueTable make(std::uint64_t record_count) {
    DenseValueTable output;
    output.values_.reserve(static_cast<std::size_t>(record_count));
    for (std::uint64_t address = 0; address < record_count; ++address) {
      output.values_.push_back(synthetic_value_at(address));
    }
    return output;
  }

  [[nodiscard]] std::uint64_t value_at(std::uint64_t address) const {
    return values_.at(static_cast<std::size_t>(address));
  }

  [[nodiscard]] StorageMetrics metrics() const {
    return {.record_count = values_.size(),
            .value_payload_bytes =
                values_.size() * static_cast<std::uint64_t>(sizeof(std::uint64_t)),
            .address_metadata_bytes = 0,
            .container_bytes =
                values_.capacity() * static_cast<std::uint64_t>(sizeof(std::uint64_t))};
  }

 private:
  std::vector<std::uint64_t> values_;
};

struct AddressValuePair {
  std::uint64_t address = 0;
  std::uint64_t value = 0;
};

class SortedPairTable {
 public:
  [[nodiscard]] static SortedPairTable make(std::uint64_t record_count) {
    SortedPairTable output;
    output.values_.reserve(static_cast<std::size_t>(record_count));
    for (std::uint64_t address = 0; address < record_count; ++address) {
      output.values_.push_back({.address = address, .value = synthetic_value_at(address)});
    }
    return output;
  }

  [[nodiscard]] std::uint64_t value_at(std::uint64_t address) const {
    const auto found = std::lower_bound(
        values_.begin(), values_.end(), address,
        [](const AddressValuePair& entry, std::uint64_t key) {
          return entry.address < key;
        });
    if (found == values_.end() || found->address != address) {
      throw std::out_of_range("address is not present in sorted pair table");
    }
    return found->value;
  }

  [[nodiscard]] StorageMetrics metrics() const {
    return {.record_count = values_.size(),
            .value_payload_bytes =
                values_.size() * static_cast<std::uint64_t>(sizeof(std::uint64_t)),
            .address_metadata_bytes =
                values_.size() * static_cast<std::uint64_t>(sizeof(std::uint64_t)),
            .container_bytes =
                values_.capacity() * static_cast<std::uint64_t>(sizeof(AddressValuePair))};
  }

 private:
  std::vector<AddressValuePair> values_;
};

class HashValueTable {
 public:
  [[nodiscard]] static HashValueTable make(std::uint64_t record_count) {
    HashValueTable output;
    output.values_.reserve(static_cast<std::size_t>(record_count));
    for (std::uint64_t address = 0; address < record_count; ++address) {
      output.values_.emplace(address, synthetic_value_at(address));
    }
    return output;
  }

  [[nodiscard]] std::uint64_t value_at(std::uint64_t address) const {
    return values_.at(address);
  }

  [[nodiscard]] StorageMetrics metrics() const {
    using Entry = std::unordered_map<std::uint64_t, std::uint64_t>::value_type;
    return {.record_count = values_.size(),
            .value_payload_bytes =
                values_.size() * static_cast<std::uint64_t>(sizeof(std::uint64_t)),
            .address_metadata_bytes =
                values_.size() * static_cast<std::uint64_t>(sizeof(std::uint64_t)),
            .container_bytes =
                values_.size() * static_cast<std::uint64_t>(sizeof(Entry)) +
                values_.bucket_count() * static_cast<std::uint64_t>(sizeof(void*))};
  }

 private:
  std::unordered_map<std::uint64_t, std::uint64_t> values_;
};

class FormulaRecipeTable {
 public:
  [[nodiscard]] static FormulaRecipeTable make(std::uint64_t record_count) {
    FormulaRecipeTable output;
    output.record_count_ = record_count;
    output.specification_ = make_power_offset_spec(2, 100, 1);
    return output;
  }

  [[nodiscard]] bool contains(std::uint64_t address) const {
    return address < record_count_;
  }

  [[nodiscard]] std::optional<std::string> symbolic_value_at(
      std::uint64_t address) const {
    if (!contains(address)) return std::nullopt;
    return specification_.scale.convert_to<std::string>() + "*(" +
           specification_.base.convert_to<std::string>() + "^" +
           std::to_string(address) + "-1)";
  }

  [[nodiscard]] const FormulaSpec& specification() const {
    return specification_;
  }

  [[nodiscard]] StorageMetrics metrics() const {
    return {.record_count = record_count_,
            .value_payload_bytes = 0,
            .address_metadata_bytes = 0,
            .container_bytes = sizeof(*this)};
  }

 private:
  std::uint64_t record_count_ = 0;
  FormulaSpec specification_{};
};

}  // namespace bruce::lhme::benchmark::table
