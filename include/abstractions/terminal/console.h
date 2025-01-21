#pragma once

#include <fmt/format.h>
#include <fmt/std.h>

#include <string>

namespace abstractions::terminal {

/// @brief A simple console printer.
///
/// This is mainly to make all of the program output look "consistent" for all
/// standard console output.
class Console {
public:
    /// @brief Create a new Console instance.
    /// @param name value shown at the start of the console output
    Console(const std::string &name);

    /// @brief Create a new console instance.
    /// @param name value shown at the start of the console output
    /// @param fmt the format string used for the output prefix
    Console(const std::string &name, const std::string &fmt);

    /// @brief Get the prefix shown at the start of a line.
    std::string_view Prefix() const;

    /// @brief Show or hide the prefix string show at the start of a console line.
    void ShowPrefix(bool show);

    /// @brief Print a message to stdout.
    /// @param msg message string
    void Print(const std::string &msg) const {
        PrintToStdout(msg);
    }

    /// @brief Print a formatted messag to stdout
    /// @param fmt format string
    /// @param args values for the format string
    template <typename... T>
    void Print(fmt::format_string<T...> fmt, T &&...args) const {
        auto format_args = fmt::make_format_args(args...);
        auto formatted = fmt::vformat(fmt, format_args);
        PrintToStdout(formatted);
    }

    /// @brief Creates a separator in the console output.
    /// @param length separator length
    /// @param separator separator character
    void Separator(int length = 10, const std::string &separator = "\u2500") const;

private:
    void PrintToStdout(fmt::string_view msg) const;

    std::string _prefix;
    std::string _separator;
    bool _show_prefix;
};

}  // namespace abstractions::terminal
