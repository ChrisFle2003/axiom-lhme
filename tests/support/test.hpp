#pragma once

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace test {

using TestFunction = std::function<void()>;

struct Case {
  std::string name;
  TestFunction function;
};

inline std::vector<Case>& cases() {
  static std::vector<Case> registered_cases;
  return registered_cases;
}

class Registration {
 public:
  Registration(std::string name, TestFunction function) {
    cases().push_back({std::move(name), std::move(function)});
  }
};

}  // namespace test

#define TEST(name) \
  static void name(); \
  static test::Registration name##_registration{#name, name}; \
  static void name()

#define CHECK(condition) \
  do { \
    if (!(condition)) { \
      throw std::runtime_error(std::string{"CHECK failed: "} + #condition); \
    } \
  } while (false)
