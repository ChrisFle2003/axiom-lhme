#include <string>

#include "demo_report.hpp"
#include "bruce/lhme/types.hpp"

int main() {
  return bruce::lhme::demo::status_name(bruce::lhme::Status::BudgetExceeded) ==
                 "BudgetExceeded"
             ? 0
             : 1;
}
