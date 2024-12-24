#pragma once

#include <abstractions/math/random.h>
#include <abstractions/math/types.h>
#include <abstractions/errors.h>

#include <tuple>

namespace abstractions {

/// @brief A collection of shape parameter vectors.
/// @tparam D number of dimensions needed to describe a shape
template<int D>
struct ShapeCollection
{
    static_assert(D > 0, "Shape dimensions must be greater than zero.");

    /// @brief Number of dimensions just to represent the shape.
    static constexpr int ShapeDimensions = D;

    /// @brief Total number of dimensions in a shape vector, including colour.
    static constexpr int TotalDimensions = D + 4;

    /// @brief Create a new ShapeCollection instance.
    /// @param num_shapes number of shapes in the collection
    ShapeCollection(int num_shapes)
    {
        abstractions_assert(num_shapes > 0);
        Params = Matrix::Zero(num_shapes, TotalDimensions);
    }

    /// @brief A `NxD` matrix with the `N` shape vectors, each `D` dimensions in
    ///      length.
    Matrix Params;

    /// @brief Get a view of the parameters matrix as a single vector.
    auto AsVector() const
    {
        return Params.reshaped();
    }

    /// @brief Only get the submatrix containing the shape parameters.
    auto ShapeParameters()
    {
        return Params.leftCols<D>();
    }

    /// @brief Only get the submatrix containing the shape colours.
    auto ColourValues()
    {
        return Params.rightCols<4>();
    }

    /// @brief Get the number of shapes in the collection.
    /// @return
    int NumShapes() const
    {
        return Params.rows();
    }
};

/// @brief Store circles as `(x,y,r)` points, where `r` is the radius.
using CircleCollection = ShapeCollection<3>;

/// @brief Store triangles as a set of three points in `(x1,y1,x2,y2,x3,y3)`
///     format.
using TriangleCollection = ShapeCollection<6>;

/// @brief Store rectangles as set of corners in a `(x1,y1,x2,y2)` format.
using RectangleCollection = ShapeCollection<4>;

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
    CircleCollection RandomCircles(const int num);

    /// @brief Generate a set of random trianges with random colours.
    /// @param num number of triangles
    /// @return shape parameters matrix
    TriangleCollection RandomTriangles(const int num);

    /// @brief Generate a set of random rectangles with random colours.
    /// @param num number of rectangles
    /// @return shape parameters matrix
    RectangleCollection RandomRectangles(const int num);

    /// @brief The aspect ratio the generator is configured for.
    double AspectRatio() const {
        return _aspect_ratio;
    }

private:
    const double _aspect_ratio;
    UniformDistribution<> _dist;
};

}  // namespace abstractions
