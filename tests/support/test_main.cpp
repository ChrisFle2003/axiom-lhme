#include "test.hpp"

int main() {
  int failures = 0;
  for (const auto& test_case : test::cases()) {
    try {
      test_case.function();
      std::cout << "PASS " << test_case.name << '\n';
    } catch (const std::exception& error) {
      ++failures;
      std::cerr << "FAIL " << test_case.name << ": " << error.what() << '\n';
    }
  }
  return failures == 0 ? 0 : 1;
}
