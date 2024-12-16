#include <abstractions/utilities.h>

#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/ostream.h>
#include <fmt/std.h>

#include <iostream>
#include <filesystem>

namespace abstractions
{

#ifdef ABSTRACTIONS_ENABLE_ASSERTS

void _assert(const bool cond, const std::string &cond_str, const std::source_location loc)
{
    if (cond) {
        return;
    }

    auto source_file = std::filesystem::path(loc.file_name());
    auto header = fmt::format("{}\n{}", fmt::styled("Assertion Failed!", fmt::emphasis::bold | fmt::fg(fmt::color::red)), fmt::styled("--", fmt::emphasis::faint));
    auto assert_cond = fmt::format("{}", cond_str);
    auto assert_func = fmt::format(fmt::emphasis::italic, "{}", loc.function_name());
    auto assert_loc = fmt::format("{}:{}", source_file.filename(), loc.line());

    fmt::println(std::cerr, "{}", header);
    fmt::println(std::cerr, "Condition        ->  {}", assert_cond);
    fmt::println(std::cerr, "Calling Function ->  {}", assert_func);
    fmt::println(std::cerr, "Location         ->  {}", assert_loc);
    fmt::println(std::cerr, "");

    throw errors::AbstractionsError("Assert failed.");
    // std::abort();
}

#endif // ABSTRACTIONS_ENABLE_ASSERTS

}
