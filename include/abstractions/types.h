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
using Error = std::optional<std::string>;

/// @brief Stores the expected result of an operation.
/// @tparam T expected result type
///
/// If an error occurs then this will contain an Error value instead of the
/// expected result type.
template <typename T>
using Expected = std::expected<T, Error>;

/// @brief Basic definition of an `MxN` matrix.
using Matrix = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

/// @brief Basic definition of a column vector, or a `Nx1` matrix.
using ColumnVector = Eigen::Matrix<double, Eigen::Dynamic, 1>;

/// @brief Basic defintion of a row vector, or a `1xN` matrix.
using RowVector = Eigen::Matrix<double, 1, Eigen::Dynamic>;

/// @brief An Eigen-friend way to pass a Matrix by reference.
///
/// Use this instead of `Matrix &`.
using MatrixRef = Eigen::Ref<Matrix>;

/// @brief An Eigen-friendly constant reference to a Matrix.
///
/// Use this instead of `const Matrix &`.
using ConstMatrixRef = const Eigen::Ref<const Matrix>&;

/// @brief An Eigen-friend constant reference to a ColumnVector.
///
/// Use this instead of `const ColumnVector &`
using ConstColumnVectorRef = const Eigen::Ref<const ColumnVector>&;

/// @brief An Eigen-friend constant reference to a RowVector.
///
/// Use this instead of `const RowVector &`.
using ConstRowVectorRef = const Eigen::Ref<const RowVector>&;

}  // namespace abstractions
