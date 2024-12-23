#include <abstractions/errors.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/std.h>

#include <filesystem>
#include <iostream>

namespace abstractions {

#ifdef ABSTRACTIONS_ENABLE_ASSERTS

void _assert(const bool cond, const std::string &cond_str, const bool throw_only,
             const std::source_location loc) {
    if (cond) {
        return;
    }

    if (throw_only) {
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

    throw errors::AbstractionsError("Assert failed.");
    // std::abort();
}

void _assert(const Error &error, const bool throw_only, const std::source_location loc)
{
    _assert(!error.has_value(), error.value_or("No error"), throw_only, loc);
}

#endif  // ABSTRACTIONS_ENABLE_ASSERTS

}  // namespace abstractions
