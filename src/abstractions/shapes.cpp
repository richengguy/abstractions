#include "abstractions/shapes.h"

#include <abstractions/math/matrices.h>

namespace abstractions
{

ShapeGenerator::ShapeGenerator(const int width, const int height, Prng<> &prng)
    : ShapeGenerator(static_cast<double>(width) / static_cast<double>(height), prng)
{

}

ShapeGenerator::ShapeGenerator(const double aspect, Prng<> &prng)
    : _aspect_ratio{aspect},
      _dist{prng}
{

}

Matrix ShapeGenerator::RandomCircles(const int num)
{
    return RandomMatrix(num, 7, _dist);
}

Matrix ShapeGenerator::RandomTriangles(const int num)
{
    return RandomMatrix(num, 10, _dist);
}

Matrix ShapeGenerator::RandomRectangles(const int num)
{
    return RandomMatrix(num, 8, _dist);
}

}
