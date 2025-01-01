#include <abstractions/shapes.h>
#include <doctest/doctest.h>

#include <tuple>

#include "support.h"

using namespace abstractions;

namespace {

template <int N>
void InitShapeCollection(ShapeCollection<N> &collection, int start)
{
    int index = start + 1;
    for (int i = 0; i < collection.NumShapes(); i++)
    {
        for (int j = 0; j < ShapeCollection<N>::TotalDimensions; j++)
        {
            collection.Params(i, j) = index++;
        }
    }
}

}

TEST_SUITE_BEGIN("shapes");

TEST_CASE("Shape collections can be empty.") {
    ShapeCollection<2> collection;
    REQUIRE(collection.Empty());
}

TEST_CASE("Shape collections are initialized correctly.") {
    ShapeCollection<2> collection(5);
    REQUIRE(collection.NumShapes() == 5);
    REQUIRE(ShapeCollection<2>::ShapeDimensions == 2);
    REQUIRE(ShapeCollection<2>::TotalDimensions == 6);
}

TEST_CASE("Can get the individual parts of the parameters matrix.") {
    ShapeCollection<1> collection(2);
    collection.Params(0, 0) = 1;
    collection.Params(0, 1) = 2;
    collection.Params(0, 2) = 3;
    collection.Params(0, 3) = 4;
    collection.Params(0, 4) = 5;
    collection.Params(1, 0) = 0;
    collection.Params(1, 1) = 0;
    collection.Params(1, 2) = 0;
    collection.Params(1, 3) = 0;
    collection.Params(1, 4) = 0;

    Matrix shape = collection.ShapeParameters();
    REQUIRE(shape.cols() == 1);
    REQUIRE(shape.rows() == 2);
    CHECK(shape(0, 0) == 1);
    CHECK(shape(1, 0) == 0);

    Matrix colour = collection.ColourValues();
    REQUIRE(colour.cols() == 4);
    REQUIRE(colour.rows() == 2);
    CHECK(colour(0, 0) == 2);
    CHECK(colour(0, 1) == 3);
    CHECK(colour(0, 2) == 4);
    CHECK(colour(0, 3) == 5);
    CHECK(colour(1, 0) == 0);
    CHECK(colour(1, 1) == 0);
    CHECK(colour(1, 2) == 0);
    CHECK(colour(1, 3) == 0);
}

TEST_CASE("Can pack individual shape collections.")
{
    CircleCollection empty_circles;
    RectangleCollection empty_rectangles;
    TriangleCollection empty_triangles;

    CircleCollection circles(2);
    InitShapeCollection(circles, 0);

    RectangleCollection rectangles(2);
    InitShapeCollection(rectangles, 50);

    TriangleCollection triangles(2);
    InitShapeCollection(triangles, 100);

    // Doing the weird test parameterization that Doctest recommends.

    using CollectionTuple = std::tuple<CircleCollection, RectangleCollection, TriangleCollection>;
    CollectionTuple shape_collections;
    std::vector<CollectionTuple> test_set{
        {circles, empty_rectangles, empty_triangles},
        {empty_circles, rectangles, empty_triangles},
        {empty_circles, empty_rectangles, triangles},
        {circles, empty_rectangles, triangles},
        {empty_circles, rectangles, triangles},
        {circles, rectangles, triangles},
    };

    ABSTRACTIONS_PARAMETERIZED_TEST(shape_collections, test_set);

    auto [test_circles, test_rectangles, test_triangles] = shape_collections;

    bool expect_circles = !test_circles.Empty();
    bool expect_rectangles = !test_rectangles.Empty();
    bool expect_triangles = !test_triangles.Empty();

    PackedShapeCollection packed(test_circles, test_rectangles, test_triangles);

    CHECK((packed.Shapes() & AbstractionShape::Circles) == expect_circles);
    CHECK((packed.Shapes() & AbstractionShape::Rectangles) == expect_rectangles);
    CHECK((packed.Shapes() & AbstractionShape::Triangles) == expect_triangles);

    CHECK(packed.Circles().Params == test_circles.Params);
    CHECK(packed.Rectangles().Params == test_rectangles.Params);
    CHECK(packed.Triangles().Params == test_triangles.Params);
}

TEST_CASE("Assert when all shape collections are empty.")
{
    CircleCollection empty_circles;
    RectangleCollection empty_rectangles;
    TriangleCollection empty_triangles;

    REQUIRE_THROWS_AS(PackedShapeCollection(empty_circles, empty_rectangles, empty_triangles), errors::AbstractionsError);
}

TEST_CASE("Assert when shapes have different sizes.")
{
    CircleCollection empty_circles;
    RectangleCollection rectangles(5);
    TriangleCollection triangles(8);

    REQUIRE_THROWS_AS(PackedShapeCollection(empty_circles, rectangles, triangles), errors::AbstractionsError);
}

TEST_SUITE_END();
