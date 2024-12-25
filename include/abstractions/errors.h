#pragma once

#include <abstractions/types.h>

#include <initializer_list>
#include <iostream>
#include <source_location>
#include <stdexcept>
#include <string>

namespace abstractions {

// The asserts can have their debug message silenced by defining
// ABSTRACTIONS_ASSERTS_THROW_ONLY.

#ifdef ABSTRACTIONS_ASSERTS_THROW_ONLY
#define __ABSTRACTIONS_THROW_ONLY true
#else
#define __ABSTRACTIONS_THROW_ONLY false
#endif

#ifdef ABSTRACTIONS_ENABLE_ASSERTS
#define abstractions_assert(cond) ::abstractions::_assert((cond), #cond, __ABSTRACTIONS_THROW_ONLY)
#define abstractions_check(err) ::abstractions::_check(err, __ABSTRACTIONS_THROW_ONLY)

void _assert(const bool cond, const std::string &cond_str, const bool throw_only,
             const std::source_location loc = std::source_location::current());

void _check(const Error &error, const bool throw_only,
            const std::source_location loc = std::source_location::current());

template <typename T>
void _check(const Expected<T> &output, const bool throw_only,
            const std::source_location loc = std::source_location::current()) {
    if (output.has_value()) {
        return;
    }
    _check(output.error(), throw_only, loc);
}
#else
#define abstractions_assert(cond)
#define abstractions_check(err) err
#endif

namespace errors {

/// @brief Use this as the return value when a operation suceeds.
static constexpr Error no_error = std::nullopt;

/// @brief Exception thrown when the abstractions library encounters a
///     non-recoverable error.
///
/// This should only be caught in a program's `main()` function to perform any
/// last minute clean-up before terminating the program.
class AbstractionsError : public std::runtime_error {
public:
    AbstractionsError(const std::string &condition, const std::source_location &location) :
        std::runtime_error("Assert failed."),
        condition{condition},
        location{location} {}

    void Print() const;

    const std::string condition;
    const std::source_location location;
};

/// @brief Creates an error when an expected value could not be returned.
/// @tparam T expected result type
/// @param err The reason why the expected value could not be returned.
/// @note This is a thin wrapper around std::unexpected.
/// @return The "unexpected" value, which will be an Error object.
template <typename T>
Expected<T> report(const char *msg) {
    return std::unexpected<Error>(msg);
}

/// @brief Creates an error when an expected value could not be returned.
/// @tparam T expected result type
/// @param err The reason why the expected value could not be returned.
/// @note This is a thin wrapper around std::unexpected.
/// @return The "unexpected" value, which will be an Error object.
template <typename T>
Expected<T> report(const std::string &msg) {
    return report<T>(msg.c_str());
}

/// @brief Creates an error when an expected value could not be returned.
/// @tparam T expected result type
/// @param err An error containing a reason for the error.
/// @note This is a thin wrapper around std::unexpected.
/// @return The "unexpected" value, which will contain the provided error.
/// @throws std::runtime_error if the error is empty
template <typename T>
Expected<T> report(const Error &err) {
    if (!err) {
        throw std::runtime_error("There is no error to report.");
    }

    return std::unexpected(err);
}

/// @brief Find any errors in a list of possible errors.
/// @param errs Set of errors
/// @return The first detected error, or a std::nullopt.
inline Error find_any(std::initializer_list<Error> errs) {
    for (const auto &e : errs) {
        if (e) {
            return e;
        }
    }

    return std::nullopt;
}

}  // namespace errors

}  // namespace abstractions
