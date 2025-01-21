#pragma once

#include <fmt/ostream.h>

#include <Eigen/Core>

namespace abstractions {
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

/// @brief An Eigen-friendly way to pass a ColumnVector by reference.
///
/// Use this instead of `ColumnVector &`.
using ColumnVectorRef = Eigen::Ref<ColumnVector>;

/// @brief An Eigen-friendly way to pass a RowVector by reference.
///
/// Use this instead of `RowVector &`.
using RowVectorRef = Eigen::Ref<RowVector>;

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

template <>
struct fmt::formatter<abstractions::Matrix> : fmt::ostream_formatter {};

template <>
struct fmt::formatter<abstractions::RowVector> : fmt::ostream_formatter {};

template <>
struct fmt::formatter<abstractions::ColumnVector> : fmt::ostream_formatter {};
