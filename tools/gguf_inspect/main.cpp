#include "command.hpp"

#include <iostream>
#include <string_view>
#include <vector>

int main(int argc, char** argv) {
  std::vector<std::string_view> arguments;
  arguments.reserve(static_cast<std::size_t>(argc));
  for (int index = 0; index < argc; ++index) arguments.emplace_back(argv[index]);
  return axiom::lhme::tool::run_gguf_inspect(arguments, std::cin, std::cout,
                                              std::cerr);
}
