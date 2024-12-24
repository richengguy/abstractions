#pragma once

#include <abstractions/math/random.h>
#include <abstractions/math/types.h>

#include <tuple>

namespace abstractions {

/// @brief Generate shape parameter matrices.
///
/// The convention for the shape parameter matrices are for each shape to be
/// represented by a single column in a `DxN` matrix, where `D` is a
/// concatentation of the shape coordinates and a 4-vector containing the
/// colour.
class ShapeGenerator {
public:
    /// @brief Create a new ShapeGenerator for a canvas of a particular size.
    /// @param width canvas width
    /// @param height canvas height
    /// @param prng random number generator
    ShapeGenerator(const int width, const int height, Prng<> &prng);

    /// @brief Create a new ShapeGenerator for a canvas with a given aspect ratio.
    /// @param aspect canvas aspect ratio (height/width)
    /// @param prng random number generator
    ShapeGenerator(const double aspect, Prng<> &prng);

    /// @brief Generate a set of random circles with random colours.
    /// @param num number of circles
    /// @return shape parameters matrix
    Matrix RandomCircles(const int num);

    /// @brief Generate a set of random trianges with random colours.
    /// @param num number of triangles
    /// @return shape parameters matrix
    Matrix RandomTriangles(const int num);

    /// @brief Generate a set of random rectangles with random colours.
    /// @param num number of rectangles
    /// @return shape parameters matrix
    Matrix RandomRectangles(const int num);

    /// @brief The aspect ratio the generator is configured for.
    double AspectRatio() const {
        return _aspect_ratio;
    }

private:
    const double _aspect_ratio;
    UniformDistribution<> _dist;
};

}  // namespace abstractions
