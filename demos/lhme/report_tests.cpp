#include <string>

#include "demo_report.hpp"
#include "axiom/lhme/types.hpp"

int main() {
  return axiom::lhme::demo::status_name(axiom::lhme::Status::BudgetExceeded) ==
                 "BudgetExceeded"
             ? 0
             : 1;
}
