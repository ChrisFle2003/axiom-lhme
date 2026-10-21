#pragma once

#include <iosfwd>
#include <span>
#include <string_view>

namespace bruce::lhme::tool {

[[nodiscard]] int run_gguf_inspect(std::span<const std::string_view> arguments,
                                   std::istream& input, std::ostream& output,
                                   std::ostream& error);

}  // namespace bruce::lhme::tool
