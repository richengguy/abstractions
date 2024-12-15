#pragma once

#include <abstractions/math/types.h>

#include <Eigen/Core>
#include <expected>
#include <optional>
#include <string>

namespace abstractions {

/// @brief Common error type
///
/// This will be empty if the function or method returning the error type
/// completed successfully.  Otherwise it will return a string describing the
/// reason for the error.
using Error = std::optional<std::string>;

/// @brief Stores the expected result of an operation.
/// @tparam T expected result type
///
/// If an error occurs then this will contain an Error value instead of the
/// expected result type.
template <typename T>
using Expected = std::expected<T, Error>;

}  // namespace abstractions
