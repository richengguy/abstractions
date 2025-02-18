#include "abstractions/render/shapes.h"

#include <abstractions/math/matrices.h>
#include <fmt/ranges.h>

#include <vector>

namespace abstractions::render {

namespace {

template <typename S, typename T>
auto ReshapeAsParamsMatrix(const Eigen::MatrixBase<T> &vector, const int start_index,
                           const int num_shapes) {
    const int total_length = S::TotalDimensions * num_shapes;
    return vector.segment(start_index, total_length)
        .reshaped(S::TotalDimensions, num_shapes)
        .transpose();
}

}  // namespace

ShapeGenerator::ShapeGenerator(const int width, const int height, Prng<> prng) :
    ShapeGenerator(static_cast<double>(width) / static_cast<double>(height), prng) {}

ShapeGenerator::ShapeGenerator(const double aspect, Prng<> prng) :
    _aspect_ratio{aspect},
    _dist{prng} {}

CircleCollection ShapeGenerator::RandomCircles(const int num) {
    CircleCollection collection(num);
    RandomMatrix(collection.Params, _dist);

    // The circle's initial radii is reduced since it's very easy for a single
    // circle to cover the entire image (the RNG just has to set the radius to
    // '1' to make it twice the size of the image).
    collection.Params.col(2) *= 0.25;

    return collection;
}

TriangleCollection ShapeGenerator::RandomTriangles(const int num) {
    TriangleCollection collection(num);
    RandomMatrix(collection.Params, _dist);
    return collection;
}

RectangleCollection ShapeGenerator::RandomRectangles(const int num) {
    RectangleCollection collection(num);
    RandomMatrix(collection.Params, _dist);
    return collection;
}

PackedShapeCollection::PackedShapeCollection() :
    _collection_size{0},
    _circles{},
    _rectangles{},
    _triangles{} {}

PackedShapeCollection::PackedShapeCollection(Options<AbstractionShape> shapes,
                                             ConstRowVectorRef params) {
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

    if (has_circles) {
        total_shape_params += CircleCollection::TotalDimensions;
    }

    if (has_rects) {
        total_shape_params += RectangleCollection::TotalDimensions;
    }

    if (has_triangles) {
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
    if (has_circles) {
        _circles.Params =
            ReshapeAsParamsMatrix<CircleCollection>(params, start_index, _collection_size);
        start_index += _circles.Params.size();
    }

    if (has_rects) {
        _rectangles.Params =
            ReshapeAsParamsMatrix<RectangleCollection>(params, start_index, _collection_size);
        start_index += _rectangles.Params.size();
    }

    if (has_triangles) {
        _triangles.Params =
            ReshapeAsParamsMatrix<TriangleCollection>(params, start_index, _collection_size);
        start_index += _triangles.Params.size();
    }
}

PackedShapeCollection::PackedShapeCollection(Options<AbstractionShape> shapes, int num_shapes) {
    int num_circles = shapes & AbstractionShape::Circles ? num_shapes : 0;
    int num_rectangles = shapes & AbstractionShape::Rectangles ? num_shapes : 0;
    int num_triangles = shapes & AbstractionShape::Triangles ? num_shapes : 0;

    _circles = CircleCollection(num_circles);
    _rectangles = RectangleCollection(num_rectangles);
    _triangles = TriangleCollection(num_triangles);
}

PackedShapeCollection::PackedShapeCollection(const CircleCollection &circles,
                                             const RectangleCollection &rectangles,
                                             const TriangleCollection &triangles) {
    int num_circles = circles.NumShapes();
    int num_rectangles = rectangles.NumShapes();
    int num_triangles = triangles.NumShapes();

    int num_shapes = 0;
    if (num_circles != 0) {
        num_shapes = num_circles;
    } else if (num_rectangles != 0) {
        num_shapes = num_rectangles;
    } else if (num_triangles != 0) {
        num_shapes = num_triangles;
    }

    // Ensure the shape collections are either empty or the same size.
    abstractions_assert(num_circles == 0 || num_circles == num_shapes);
    abstractions_assert(num_rectangles == 0 || num_rectangles == num_shapes);
    abstractions_assert(num_triangles == 0 || num_triangles == num_shapes);

    _circles = circles;
    _rectangles = rectangles;
    _triangles = triangles;
}

Options<AbstractionShape> PackedShapeCollection::Shapes() const {
    Options<AbstractionShape> shapes;

    if (!_circles.Empty()) {
        shapes.Set(AbstractionShape::Circles);
    }

    if (!_rectangles.Empty()) {
        shapes.Set(AbstractionShape::Rectangles);
    }

    if (!_triangles.Empty()) {
        shapes.Set(AbstractionShape::Triangles);
    }

    return shapes;
}

int PackedShapeCollection::CollectionSize() const {
    return _collection_size;
}

int PackedShapeCollection::TotalDimensions() const {
    auto shapes = Shapes();
    int total_dimensions = 0;
    total_dimensions += shapes & AbstractionShape::Circles ? CircleCollection::TotalDimensions : 0;
    total_dimensions +=
        shapes & AbstractionShape::Rectangles ? RectangleCollection::TotalDimensions : 0;
    total_dimensions +=
        shapes & AbstractionShape::Triangles ? TriangleCollection::TotalDimensions : 0;
    return total_dimensions;
}

RowVector PackedShapeCollection::AsPackedVector() const {
    const int circles_size = _circles.Params.size();
    const int rects_size = _rectangles.Params.size();
    const int triangles_size = _triangles.Params.size();
    const int total_size = circles_size + rects_size + triangles_size;

    RowVector packed(total_size);
    int start_index = 0;

    if (circles_size > 0) {
        packed.segment(start_index, circles_size) = _circles.AsVector();
        start_index += circles_size;
    }

    if (rects_size > 0) {
        packed.segment(start_index, rects_size) = _rectangles.AsVector();
        start_index += rects_size;
    }

    if (triangles_size > 0) {
        packed.segment(start_index, triangles_size) = _triangles.AsVector();
    }

    return packed;
}

}  // namespace abstractions::render

using namespace fmt;
using namespace abstractions;
using namespace abstractions::render;

format_context::iterator formatter<AbstractionShape>::format(AbstractionShape shape,
                                                             format_context &ctx) const {
    string_view name = "undefined";
    switch (shape) {
        case AbstractionShape::Circles:
            name = "Circles";
            break;
        case AbstractionShape::Rectangles:
            name = "Rectangles";
            break;
        case AbstractionShape::Triangles:
            name = "Triangles";
            break;
    }
    return formatter<string_view>::format(name, ctx);
}

format_context::iterator formatter<Options<AbstractionShape>>::format(
    Options<AbstractionShape> options, format_context &ctx) const {
    std::vector<AbstractionShape> selected;
    if (options & AbstractionShape::Circles) {
        selected.push_back(AbstractionShape::Circles);
    }

    if (options & AbstractionShape::Rectangles) {
        selected.push_back(AbstractionShape::Rectangles);
    }

    if (options & AbstractionShape::Triangles) {
        selected.push_back(AbstractionShape::Triangles);
    }

    auto out = fmt::format("{{ {} }}", fmt::join(selected, " "));
    return formatter<string_view>::format(out, ctx);
}
