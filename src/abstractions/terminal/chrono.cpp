#include "abstractions/terminal/chrono.h"

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>

namespace abstractions::terminal {

namespace {

template <typename T>
std::string ToDuration(detail::Duration duration) {
    return fmt::format("{}", std::chrono::duration_cast<T>(duration));
}

}  // namespace

std::string FormatDuration(detail::Duration duration) {
    using namespace std::chrono_literals;

    if (duration > 1s) {
        return ToDuration<std::chrono::seconds>(duration);
    }

    if (duration > 1ms) {
        return ToDuration<std::chrono::milliseconds>(duration);
    }

    if (duration > 1us) {
        return ToDuration<std::chrono::microseconds>(duration);
    }

    return ToDuration<std::chrono::microseconds>(duration);
}

}  // namespace abstractions::terminal

fmt::format_context::iterator fmt::formatter<abstractions::OperationTiming>::format(
    const abstractions::OperationTiming &timing, fmt::format_context &ctx) const {
    auto stats = timing.GetTiming();
    std::string output =
        fmt::format("{} \u00b1 {}", abstractions::terminal::FormatDuration(stats.mean),
                    abstractions::terminal::FormatDuration(stats.stddev));
    return fmt::formatter<std::string>::format(output, ctx);
}
