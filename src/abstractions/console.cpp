#include "abstractions/console.h"

#include <fmt/color.h>

#include <algorithm>

namespace abstractions {

Console::Console(const std::string &name) :
    Console(name, "[{}]") {}

Console::Console(const std::string &name, const std::string &fmt) {
    auto styled_name =
        fmt::format("{}", fmt::styled(name, fmt::emphasis::italic | fmt::emphasis::faint));
    _prefix = fmt::format(fmt::runtime(fmt), styled_name);
}

void Console::PrintToStdout(fmt::string_view msg, fmt::format_args args) const {
    fmt::println("{} {}", _prefix, fmt::vformat(msg, args));
}

void Console::Separator(int length, const std::string &separator) const {
    auto sepfmt = fmt::format("{{:{}^{{}}}}", separator);
    auto sepstr = fmt::format(fmt::runtime(sepfmt), "", length);
    PrintToStdout(sepstr, fmt::format_args());
}

}  // namespace abstractions
