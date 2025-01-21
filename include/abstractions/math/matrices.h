#pragma once

#include <abstractions/math/random.h>
#include <abstractions/math/types.h>

#include <random>
#include <type_traits>

namespace abstractions {

/// @brief Clamp the values in a matrix or array to be in some range, with the
///     default being `[0,1]`.
/// @tparam M Eigen matrix type
/// @param matrix matrix or matrix expression
/// @param min minimum vlaue
/// @param max maximum value
/// @return clamped values
template <typename M>
auto ClampValues(const Eigen::MatrixBase<M> &matrix, double min = 0, double max = 1) {
    return matrix.cwiseMin(max).cwiseMax(min);
}

/// @brief Rescale the matrix so the values along a column are between 0 and 1.
/// @tparam M Eigen matrix type
/// @param matrix matrix or matrix expression
/// @return rescaled matrix
template <typename M>
Matrix RescaleValuesColumnWise(const Eigen::MatrixBase<M> &matrix) {
    Eigen::Matrix<typename M::Scalar, 1, Eigen::Dynamic> min_values =
        matrix.array().colwise().minCoeff();
    Eigen::Matrix<typename M::Scalar, 1, Eigen::Dynamic> max_values =
        matrix.array().colwise().maxCoeff();
    return (matrix.rowwise() - min_values).array().rowwise() / (max_values - min_values).array();
}

/// @brief Initialize a matrix with random values.
/// @tparam G Prng generator type
/// @tparam D statistical distribution type
/// @param rows number of rows
/// @param cols number of colums
/// @param distribution the statistical distribution to use
/// @return a `rows x cols` matrix with each value distributed according to the
///     provided statistical distribution
/// @see NormalDistribution
/// @see UniformDistribution
template <typename G, typename D>
Matrix RandomMatrix(int rows, int cols, Distribution<G, D> &distribution) {
    static_assert(std::is_same<Matrix::Scalar, typename D::result_type>::value,
                  "Matrix type and distribution type must match.");
    auto sampler = [&]() { return distribution.Sample(); };
    return Matrix::NullaryExpr(rows, cols, sampler);
}

/// @brief Initialize a matrix with random values, in-place.
/// @tparam G Prng generator type
/// @tparam D statistical distribution type
/// @param[out] matrix matrix to initialize
/// @param distribution the statistical distribution to use
/// @see NormalDistribution
/// @see UniformDistribution
template <typename G, typename D>
void RandomMatrix(MatrixRef matrix, Distribution<G, D> &distribution) {
    static_assert(std::is_same<Matrix::Scalar, typename D::result_type>::value,
                  "Matrix type and distribution type must match.");
    const int rows = matrix.rows();
    const int cols = matrix.cols();
    auto sampler = [&]() { return distribution.Sample(); };
    matrix = Matrix::NullaryExpr(rows, cols, sampler);
}

}  // namespace abstractions
