#include <abstractions/render/shapes.h>
#include <doctest/doctest.h>

TEST_SUITE_BEGIN("shapes");

TEST_CASE("Shape collections are initialized correctly.") {
    using abstractions::ShapeCollection;

    ShapeCollection<2> collection(5);
    REQUIRE(collection.NumShapes() == 5);
    REQUIRE(ShapeCollection<2>::ShapeDimensions == 2);
    REQUIRE(ShapeCollection<2>::TotalDimensions == 6);
}

TEST_CASE("Can get the individual parts of the parameters matrix.") {
    using abstractions::Matrix;
    using abstractions::ShapeCollection;

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

TEST_SUITE_END();
