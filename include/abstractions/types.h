#pragma once

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
using error_t = std::optional<std::string>;

/// @brief Stores the expected result of an operation.
/// @tparam T expected result type
///
/// If an error occurs then this will contain an error_t value instead of the
/// expected result type.
template <typename T>
using expected_t = std::expected<T, error_t>;

/// @brief Basic definition of an `MxN` matrix.
using matrix_t = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

/// @brief Basic definition of a column vector, or a `Nx1` matrix.
using col_vec_t = Eigen::Matrix<double, Eigen::Dynamic, 1>;

/// @brief Basic defintion of a row vector, or a `1xN` matrix.
using row_vec_t = Eigen::Matrix<double, 1, Eigen::Dynamic>;

/// @brief An Eigen-friend way to pass a matrix_t by reference.
///
/// Use this instead of `matrix_t &`.
using matrix_ref_t = Eigen::Ref<matrix_t>;

/// @brief An Eigen-friendly constant reference to a matrix_t.
///
/// Use this instead of `const matrix_t &`.
using const_matrix_ref_t = const Eigen::Ref<const matrix_t>&;

/// @brief An Eigen-friend constant reference to a col_vec_t.
///
/// Use this instead of `const col_vec_t &`
using const_col_vec_ref_t = const Eigen::Ref<const col_vec_t>&;

/// @brief An Eigen-friend constant reference to a row_vec_t.
///
/// Use this instead of `const row_vec_t &`.
using const_row_vec_ref_t = const Eigen::Ref<const row_vec_t>&;

}  // namespace abstractions
