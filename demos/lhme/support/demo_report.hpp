#pragma once

#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

#include "axiom/lhme/types.hpp"

namespace axiom::lhme::demo {

inline std::string_view status_name(Status status) {
  switch (status) {
    case Status::Confirmed: return "Confirmed";
    case Status::Rejected: return "Rejected";
    case Status::BudgetExceeded: return "BudgetExceeded";
    case Status::Cancelled: return "Cancelled";
    case Status::InvalidArgument: return "InvalidArgument";
    case Status::UnsupportedMode: return "UnsupportedMode";
    case Status::InternalError: return "InternalError";
  }
  return "Unknown";
}

class Report {
 public:
  void heading(std::string_view title) { std::cout << "\n== " << title << " ==\n"; }

  void row(std::string_view label, std::string_view value) {
    std::cout << label << " | " << value << '\n';
  }

  template <typename T>
  void expect_equal(std::string_view label, const T& expected, const T& actual) {
    if (expected == actual) return;
    ++failures_;
    std::cout << "FAIL " << label << ": expected=" << display(expected)
              << " actual=" << display(actual) << '\n';
  }

  void expect_true(std::string_view label, bool actual) {
    expect_equal(label, true, actual);
  }

  [[nodiscard]] int finish() const {
    if (failures_ == 0) {
      std::cout << "PASS\n";
      return 0;
    }
    std::cout << "FAILURES=" << failures_ << '\n';
    return 1;
  }

 private:
  template <typename T>
  static std::string display(const T& value) {
    std::ostringstream output;
    if constexpr (std::is_enum_v<T>) {
      output << static_cast<std::underlying_type_t<T>>(value);
    } else {
      output << value;
    }
    return output.str();
  }

  std::size_t failures_ = 0;
};

}  // namespace axiom::lhme::demo
