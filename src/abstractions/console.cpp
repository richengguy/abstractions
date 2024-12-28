#include "abstractions/console.h"

#include <algorithm>
#include <fmt/color.h>

namespace abstractions
{

Console::Console(const std::string &name)
{
    auto styled_name = fmt::format("{}", fmt::styled(name, fmt::emphasis::italic | fmt::emphasis::faint));
    _prefix = fmt::format("{} ::", styled_name);
}

void Console::PrintToStdout(fmt::string_view msg, fmt::format_args args) const
{
    fmt::println("{} {}", _prefix, fmt::vformat(msg, args));
}

void Console::Separator(int length, const std::string &separator) const
{
    auto sepfmt = fmt::format("{{:{}^{{}}}}", separator);

    fmt::println(fmt::runtime(sepfmt), "", length);
}

}
