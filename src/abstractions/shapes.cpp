#include "abstractions/shapes.h"

#include <abstractions/math/matrices.h>

namespace abstractions {

namespace
{

template<typename S, typename T>
auto ReshapeAsParamsMatrix(const Eigen::MatrixBase<T> &vector, const int start_index, const int num_shapes)
{
    const int total_length = S::TotalDimensions * num_shapes;
    return vector.segment(start_index, total_length).reshaped(S::TotalDimensions, num_shapes).transpose();
}

}

ShapeGenerator::ShapeGenerator(const int width, const int height, Prng<> &prng) :
    ShapeGenerator(static_cast<double>(width) / static_cast<double>(height), prng) {}

ShapeGenerator::ShapeGenerator(const double aspect, Prng<> &prng) :
    _aspect_ratio{aspect},
    _dist{prng} {}

CircleCollection ShapeGenerator::RandomCircles(const int num) {
    CircleCollection collection(num);
    RandomMatrix(collection.Params, _dist);
    collection.Params.col(0) *= _aspect_ratio;
    return collection;
}

TriangleCollection ShapeGenerator::RandomTriangles(const int num) {
    TriangleCollection collection(num);
    RandomMatrix(collection.Params, _dist);
    collection.Params.col(0) *= _aspect_ratio;
    collection.Params.col(2) *= _aspect_ratio;
    collection.Params.col(4) *= _aspect_ratio;
    return collection;
}

RectangleCollection ShapeGenerator::RandomRectangles(const int num) {
    RectangleCollection collection(num);
    RandomMatrix(collection.Params, _dist);
    collection.Params.col(0) *= _aspect_ratio;
    collection.Params.col(2) *= _aspect_ratio;
    return collection;
}

PackedShapeCollection::PackedShapeCollection(Options<AbstractionShape> shapes, ConstRowVectorRef params)
{
    const bool has_circles = shapes & AbstractionShape::Circles;
    const bool has_rects = shapes & AbstractionShape::Rectangles;
    const bool has_triangles = shapes & AbstractionShape::Triangles;

    // The params vector contains the same number of shapes for each shape type.
    // Figuring out the number of shapes is just taking the length of that
    // vector and dividing it by the length of a packed vector that only
    // contains a single shape.  The if-statements below are used to figure out
    // the length of that signal packed vector.

    int num_params = params.size();
    int total_shape_params = 0;

    if (has_circles)
    {
        total_shape_params += CircleCollection::TotalDimensions;
    }

    if (has_rects)
    {
        total_shape_params += RectangleCollection::TotalDimensions;
    }

    if (has_triangles)
    {
        total_shape_params += TriangleCollection::TotalDimensions;
    }

    // The assert checks that the predicted packed shape divides evenly into the
    // provided vector.  If it doesn't then it means there was an error of some
    // sort.

    abstractions_assert(num_params % total_shape_params == 0);
    _collection_size = num_params / total_shape_params;

    int num_circles = has_circles ? _collection_size : 0;
    int num_rects = has_rects ? _collection_size : 0;
    int num_triangles = has_triangles ? _collection_size : 0;

    _circles = CircleCollection(num_circles);
    _rectangles = RectangleCollection(num_rects);
    _triangles = TriangleCollection(num_triangles);

    int start_index = 0;
    if (has_circles)
    {
        _circles.Params = ReshapeAsParamsMatrix<CircleCollection>(params, start_index, _collection_size);
        start_index += _circles.Params.size();
    }

    if (has_rects)
    {
        _rectangles.Params = ReshapeAsParamsMatrix<RectangleCollection>(params, start_index, _collection_size);
        start_index += _rectangles.Params.size();
    }

    if (has_triangles)
    {
        _triangles.Params = ReshapeAsParamsMatrix<TriangleCollection>(params, start_index, _collection_size);
        start_index += _triangles.Params.size();
    }
}

PackedShapeCollection::PackedShapeCollection(const CircleCollection &circles, const RectangleCollection &rectangles, const TriangleCollection &triangles)
{
    int num_circles = circles.NumShapes();
    int num_rectangles = rectangles.NumShapes();
    int num_triangles = triangles.NumShapes();

    int num_shapes = 0;
    if (num_circles != 0)
    {
        num_shapes = num_circles;
    }
    else if (num_rectangles != 0)
    {
        num_shapes = num_rectangles;
    }
    else if (num_triangles != 0)
    {
        num_shapes = num_triangles;
    }

    // At least one of the shape collections must not be empty.
    abstractions_assert(num_shapes != 0);

    // Ensure the shape collections are either empty or the same size.
    abstractions_assert(num_circles == 0 || num_circles == num_shapes);
    abstractions_assert(num_rectangles == 0 || num_rectangles == num_shapes);
    abstractions_assert(num_triangles == 0 || num_triangles == num_shapes);

    _circles = circles;
    _rectangles = rectangles;
    _triangles = triangles;
}

Options<AbstractionShape> PackedShapeCollection::Shapes() const
{
    Options<AbstractionShape> shapes;

    if (!_circles.Empty())
    {
        shapes.Set(AbstractionShape::Circles);
    }

    if (!_rectangles.Empty())
    {
        shapes.Set(AbstractionShape::Rectangles);
    }

    if (!_triangles.Empty())
    {
        shapes.Set(AbstractionShape::Triangles);
    }

    return shapes;
}

int PackedShapeCollection::CollectionSize() const
{
    return _collection_size;
}

RowVector PackedShapeCollection::AsPackedVector() const
{
    const int circles_size = _circles.Params.size();
    const int rects_size = _rectangles.Params.size();
    const int triangles_size = _triangles.Params.size();
    const int total_size = circles_size + rects_size + triangles_size;

    RowVector packed(total_size);
    int start_index = 0;

    if (circles_size > 0)
    {
        packed.segment(start_index, circles_size) = _circles.AsVector();
        start_index += circles_size;
    }

    if (rects_size > 0)
    {
        packed.segment(start_index, rects_size) = _rectangles.AsVector();
        start_index += rects_size;
    }

    if (triangles_size > 0)
    {
        packed.segment(start_index, triangles_size) = _triangles.AsVector();
    }

    return packed;
}

}  // namespace abstractions
