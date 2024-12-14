#pragma once

#include <abstractions/math/random.h>
#include <abstractions/math/types.h>

#include <random>
#include <type_traits>

namespace abstractions {

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

}  // namespace abstractions
