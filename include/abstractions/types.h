#pragma once

#include <optional>
#include <string>

namespace abstractions {

/// @brief Common error type
///
/// This will be empty if the function or method returning the error type
/// completed successfully.  Otherwise it will return a string describing the
/// reason for the error.
using error_t = std::optional<std::string>;

}  // namespace abstractions
