#pragma once

#include <abstractions/types.h>

#include <exception>
#include <initializer_list>

namespace abstractions {

namespace errors {

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
