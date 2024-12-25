#include "abstractions/errors.h"

#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/std.h>

#include <filesystem>
#include <iostream>

namespace abstractions {

namespace {

void PrintMessage(const std::source_location &loc, const std::string &cond_str)
{
    auto source_file = std::filesystem::path(loc.file_name());
    auto header = fmt::format(
        "{}\n{}",
        fmt::styled("Assertion Failed!", fmt::emphasis::bold | fmt::fg(fmt::color::red)),
        fmt::styled("--", fmt::emphasis::faint));
    auto assert_cond = fmt::format("{}", cond_str);
    auto assert_func = fmt::format(fmt::emphasis::italic, "{}", loc.function_name());
    auto assert_loc = fmt::format("{}:{}", source_file.filename(), loc.line());

    fmt::println(std::cerr, "{}", header);
    fmt::println(std::cerr, "Condition        ->  {}", assert_cond);
    fmt::println(std::cerr, "Calling Function ->  {}", assert_func);
    fmt::println(std::cerr, "Location         ->  {}", assert_loc);
    fmt::println(std::cerr, "");
}

}

void errors::AbstractionsError::Print() const
{
    PrintMessage(location, condition);
}

#ifdef ABSTRACTIONS_ENABLE_ASSERTS

void _assert(const bool cond, const std::string &cond_str, const bool throw_only,
             const std::source_location loc) {
    if (cond) {
        return;
    }

    if (!throw_only) {
        PrintMessage(loc, cond_str);
    }

    throw errors::AbstractionsError(cond_str, loc);
}

void _check(const Error &error, const bool throw_only, const std::source_location loc) {
    std::string cond_str = fmt::format("Missing expected value: {}", error.value_or(""));
    _assert(!error.has_value(), cond_str, throw_only, loc);
}

#endif  // ABSTRACTIONS_ENABLE_ASSERTS

}  // namespace abstractions
