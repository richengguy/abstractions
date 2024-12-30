#pragma once

#include <fmt/format.h>
#include <fmt/std.h>

#include <string>

namespace abstractions {

/// @brief A simple console printer.
///
/// This is mainly to make all of the program output look "consistent" for all
/// standard debugging output.
class Console {
public:
    /// @brief Create a new Console instance.
    /// @param name value shown at the start of the console output
    Console(const std::string &name);

    /// @brief Create a new console instance.
    /// @param name value shown at the start of the console output
    /// @param fmt the format string used for the output prefix
    Console(const std::string &name, const std::string &fmt);

    void Print(const std::string &msg) {
        PrintToStdout(msg, fmt::format_args{});
    }

    template <typename... T>
    void Print(fmt::format_string<T...> fmt, T &&...args) const {
        PrintToStdout(fmt, fmt::make_format_args(args...));
    }

    /// @brief Creates a separator in the console output.
    /// @param length separator length
    /// @param separator separator character
    void Separator(int length = 10, const std::string &separator = "\u2500") const;

private:
    void PrintToStdout(fmt::string_view msg, fmt::format_args args) const;

    std::string _prefix;
    std::string _separator;
};

}  // namespace abstractions
