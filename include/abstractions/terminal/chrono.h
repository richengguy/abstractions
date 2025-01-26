#pragma once

#include <abstractions/profile.h>
#include <fmt/format.h>

#include <string>

namespace abstractions::terminal {

/// @brief Convert two durations into a single percentage.
/// @param value the time for a specific operation
/// @param total the total amount of time spent
/// @return a string with the formatted percentage
template <typename Rep, typename Period>
std::string ToPercentage(std::chrono::duration<Rep, Period> value,
                         std::chrono::duration<Rep, Period> total) {
    return fmt::format("{:.3}%", 100.0 * value / total);
}

/// @brief Convert a duration into a string.  This automatically sets the units
///     since a duration will often be something very small (micro or
///     nanoseconds)
/// @param duration the duration
/// @return formatted string
std::string FormatDuration(abstractions::detail::Duration duration);

}  // namespace abstractions::terminal

template <>
struct fmt::formatter<abstractions::OperationTiming> : fmt::formatter<std::string> {
    fmt::format_context::iterator format(const abstractions::OperationTiming &timing,
                                         fmt::format_context &ctx) const;
};
