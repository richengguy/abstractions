#include "support.h"

#include <cctype>
#include <ranges>

namespace cli_helpers {

void to_lower(std::string &str) {
    std::ranges::transform(std::begin(str), std::end(str), std::begin(str),
                           [](unsigned char c) -> unsigned char { return std::tolower(c); });
}

std::string to_lower(std::string_view str) {
    std::string converted(str);
    to_lower(converted);
    return converted;
}

}  // namespace cli_helpers
