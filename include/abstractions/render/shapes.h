#pragma once

#include <abstractions/errors.h>
#include <abstractions/math/random.h>
#include <abstractions/types.h>
#include <fmt/base.h>

namespace abstractions::render {

/// @brief A collection of shape parameter vectors.
/// @tparam D number of dimensions needed to describe a shape
template <int D>
struct ShapeCollection {
    static_assert(D > 0, "Shape dimensions must be greater than zero.");

    /// @brief Number of dimensions just to represent the shape.
    static constexpr int ShapeDimensions = D;

    /// @brief Total number of dimensions in a shape vector, including colour.
    static constexpr int TotalDimensions = D + 4;

    /// @brief Create an empty ShapeCollection.
    ShapeCollection() :
        ShapeCollection(0) {}

    /// @brief Create a new ShapeCollection instance.
    /// @param num_shapes number of shapes in the collection
    ShapeCollection(int num_shapes) {
        abstractions_assert(num_shapes >= 0);
        Params = Matrix::Zero(num_shapes, TotalDimensions);
    }

    /// @brief A `NxD` matrix with the `N` shape vectors, each `D` dimensions in
    ///      length.
    Matrix Params;

    /// @brief Determine if the collection is empty.
    bool Empty() const {
        return Params.size() == 0;
    }

    /// @brief Get a view of the parameters matrix as a single vector.
    auto AsVector() const {
        return Params.transpose().reshaped();
    }

    /// @brief Only get the submatrix containing the shape parameters.
    auto ShapeParameters() {
        return Params.leftCols<D>();
    }

    /// @brief Only get the submatrix containing the shape colours.
    auto ColourValues() {
        return Params.rightCols<4>();
    }

    /// @brief Get the number of shapes in the collection.
    /// @return
    int NumShapes() const {
        return Params.rows();
    }
};

/// @brief Store circles as `(x,y,r)` points, where `r` is the radius.
using CircleCollection = ShapeCollection<3>;

/// @brief Store rectangles as set of corners in a `(x1,y1,x2,y2)` format.
using RectangleCollection = ShapeCollection<4>;

/// @brief Store triangles as a set of three points in `(x1,y1,x2,y2,x3,y3)`
///     format.
using TriangleCollection = ShapeCollection<6>;

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
    ShapeGenerator(const int width, const int height, Prng<> prng);

    /// @brief Create a new ShapeGenerator for a canvas with a given aspect ratio.
    /// @param aspect canvas aspect ratio (height/width)
    /// @param prng random number generator
    ShapeGenerator(const double aspect, Prng<> prng);

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

/// @brief Available shapes for the image abstraction.
enum class AbstractionShape { Circles, Rectangles, Triangles };

ABSTRACTIONS_OPTIONS_ENUM(AbstractionShape)

/// @brief Provides access to the individual shape collections when multiple
///     collections are inside a single parameter vector.
/// @see AbstractionShape
///
/// The packed collection always stores all available shape types.  The
/// particular collection will be empty if the provided packed collections
/// configuration doesn't contain that shape.
///
/// One limitation of this structure is that each collection must contain the
/// same number of shapes.
class PackedShapeCollection {
public:
    /// @brief Create a completely empty collection.
    PackedShapeCollection();

    /// @brief Create a new packed shape collection.
    /// @param shapes shapes stored in the packed parameters vector
    /// @param params packed shape parameters vector
    PackedShapeCollection(Options<AbstractionShape> shapes, ConstRowVectorRef params);

    /// @brief Creates a new packed shape collection, allocating space to store
    ///     the requested shapes.
    /// @param shapes shape stored in the packed collection
    /// @param num_shapes the number of shapes in the individual collections
    PackedShapeCollection(Options<AbstractionShape> shapes, int num_shapes);

    /// @brief Create a new packed shape collection.
    /// @param circles set of circles
    /// @param rectangles set of rectangles
    /// @param triangles set of triangles
    ///
    /// The individual shape collections must be the same size or be empty.
    PackedShapeCollection(const CircleCollection &circles, const RectangleCollection &rectangles,
                          const TriangleCollection &triangles);

    /// @brief The options used to describe this shape collection.
    Options<AbstractionShape> Shapes() const;

    /// @brief Get the total length of a parameter vector from a shape configuration.
    /// @return total number of dimensions
    ///
    /// This is the sum of the ShapeCollection<>::TotalDimensions property for
    /// the individual collections contained in the packed shape.
    int TotalDimensions() const;

    /// @brief The size of the individual collections.
    int CollectionSize() const;

    /// @brief Convert the internal collections into their packed representation.
    /// @return a row vector with the packed parameters
    RowVector AsPackedVector() const;

    CircleCollection &Circles() {
        return _circles;
    }
    const CircleCollection &Circles() const {
        return _circles;
    }

    RectangleCollection &Rectangles() {
        return _rectangles;
    }
    const RectangleCollection &Rectangles() const {
        return _rectangles;
    }

    TriangleCollection &Triangles() {
        return _triangles;
    }
    const TriangleCollection &Triangles() const {
        return _triangles;
    }

private:
    int _collection_size;
    CircleCollection _circles;
    RectangleCollection _rectangles;
    TriangleCollection _triangles;
};

}  // namespace abstractions::render

/// @brief Custom formatter for AbstractionShape
template <>
struct fmt::formatter<abstractions::render::AbstractionShape> : fmt::formatter<fmt::string_view> {
    fmt::format_context::iterator format(abstractions::render::AbstractionShape shape,
                                         fmt::format_context &ctx) const;
};

/// @brief Custom formatter for Options<AbstractionShape>
template <>
struct fmt::formatter<abstractions::Options<abstractions::render::AbstractionShape>>
    : fmt::formatter<fmt::string_view> {
    fmt::format_context::iterator format(
        abstractions::Options<abstractions::render::AbstractionShape> options,
        fmt::format_context &ctx) const;
};
