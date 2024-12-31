#include "abstractions/shapes.h"

#include <abstractions/math/matrices.h>

namespace abstractions {

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

}  // namespace abstractions
