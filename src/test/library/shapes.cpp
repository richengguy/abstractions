#include <abstractions/render/shapes.h>
#include <doctest/doctest.h>

#include <tuple>

#include "support.h"

using namespace abstractions;
using namespace abstractions::render;

namespace {

template <int N>
void InitShapeCollection(ShapeCollection<N> &collection, int start) {
    int index = start + 1;
    for (int i = 0; i < collection.NumShapes(); i++) {
        for (int j = 0; j < ShapeCollection<N>::TotalDimensions; j++) {
            collection.Params(i, j) = index++;
        }
    }
}

}  // namespace

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

TEST_CASE("Can pack/unpack individual shape collections.") {
    constexpr int kIndexCircles = 0;
    constexpr int kIndexRectangles = 50;
    constexpr int kIndexTriangles = 100;

    CircleCollection empty_circles;
    RectangleCollection empty_rectangles;
    TriangleCollection empty_triangles;

    CircleCollection circles(2);
    InitShapeCollection(circles, kIndexCircles);

    RectangleCollection rectangles(2);
    InitShapeCollection(rectangles, kIndexRectangles);

    TriangleCollection triangles(2);
    InitShapeCollection(triangles, kIndexTriangles);

    const int dim_circles = circles.Params.size();
    const int dim_rectangles = rectangles.Params.size();
    const int dim_triangles = triangles.Params.size();

    // Doing the weird test parameterization that Doctest recommends.  The
    // 'shape_collections' variable is what's actually set by the individual
    // subtest and 'test_set' is the full list of permutations the test runs
    // through.

    using CollectionTuple = std::tuple<CircleCollection, RectangleCollection, TriangleCollection>;
    CollectionTuple shape_collections;
    // clang-format off
    std::vector<CollectionTuple> test_set{
        {circles, empty_rectangles, empty_triangles},
        {empty_circles, rectangles, empty_triangles},
        {empty_circles, empty_rectangles, triangles},
        {circles, empty_rectangles, triangles},
        {empty_circles, rectangles, triangles},
        {circles, rectangles, triangles},
    };
    // clang-format on

    ABSTRACTIONS_PARAMETERIZED_TEST(shape_collections, test_set);

    // First, see if packing works because it should be straightforward to pack
    // the different collections into a single vector.

    auto [test_circles, test_rectangles, test_triangles] = shape_collections;

    bool expect_circles = !test_circles.Empty();
    bool expect_rectangles = !test_rectangles.Empty();
    bool expect_triangles = !test_triangles.Empty();
    auto msg = fmt::format("Circles: {}, Rectangles: {}, Triangles: {}", expect_circles,
                           expect_rectangles, expect_triangles);
    INFO("Current test: ", msg);

    int total_dims = 0;
    if (expect_circles) {
        total_dims += test_circles.Params.cols();
    }

    if (expect_rectangles) {
        total_dims += test_rectangles.Params.cols();
    }

    if (expect_triangles) {
        total_dims += test_triangles.Params.cols();
    }

    PackedShapeCollection packed(test_circles, test_rectangles, test_triangles);

    CHECK((packed.Shapes() & AbstractionShape::Circles) == expect_circles);
    CHECK((packed.Shapes() & AbstractionShape::Rectangles) == expect_rectangles);
    CHECK((packed.Shapes() & AbstractionShape::Triangles) == expect_triangles);

    CHECK(packed.TotalDimensions() == total_dims);

    CHECK(packed.Circles().Params == test_circles.Params);
    CHECK(packed.Rectangles().Params == test_rectangles.Params);
    CHECK(packed.Triangles().Params == test_triangles.Params);

    RowVector packed_vector = packed.AsPackedVector();

    int start_index = 0;

    if (expect_circles) {
        CAPTURE(start_index);
        for (int i = 0; i < dim_circles; i++) {
            CHECK(packed_vector(i + start_index) == circles.AsVector()(i));
        }
        start_index += dim_circles;
    }

    if (expect_rectangles) {
        CAPTURE(start_index);
        for (int i = 0; i < dim_rectangles; i++) {
            CHECK(packed_vector(i + start_index) == rectangles.AsVector()(i));
        }
        start_index += dim_rectangles;
    }

    if (expect_triangles) {
        CAPTURE(start_index);
        for (int i = 0; i < dim_triangles; i++) {
            CHECK(packed_vector(i + start_index) == triangles.AsVector()(i));
        }
    }

    // Now, if that all works, see if unpacking works by seeing if the original
    // collections can be recovered from the packed_vector.  This is a bit more
    // complicated as it requires figuring out how many shapes are stored in
    // the packed collection.  If the packing fails, then so will the unpacking.

    PackedShapeCollection unpacked(packed.Shapes(), packed_vector);

    REQUIRE(unpacked.CollectionSize() == 2);
    REQUIRE(unpacked.Shapes() == packed.Shapes());

    CHECK(unpacked.Circles().Params == test_circles.Params);
    CHECK(unpacked.Rectangles().Params == test_rectangles.Params);
    CHECK(unpacked.Triangles().Params == test_triangles.Params);
}

TEST_CASE("Can initialize a packed shape collection with a set number of shapes.")
{
    // Doing the weird "doctest parameterized test"-thing.  'possible_shapes' is
    // the set of all shape combinations while 'current_shapes' is the currently
    // tested configuration.

    std::vector<Options<AbstractionShape>> possible_shapes{
        AbstractionShape::Circles,
        AbstractionShape::Rectangles,
        AbstractionShape::Triangles,
        AbstractionShape::Circles | AbstractionShape::Rectangles,
        AbstractionShape::Circles | AbstractionShape::Triangles,
        AbstractionShape::Rectangles | AbstractionShape::Triangles,
        AbstractionShape::Circles | AbstractionShape::Rectangles | AbstractionShape::Triangles,
    };
    Options<AbstractionShape> current_shapes;

    ABSTRACTIONS_PARAMETERIZED_TEST(current_shapes, possible_shapes);

    PackedShapeCollection packed(current_shapes, 5);
    REQUIRE(packed.Shapes() == current_shapes);

    if (current_shapes & AbstractionShape::Circles) {
        CHECK(packed.Circles().NumShapes() == 5);
    }
    else
    {
        CHECK(packed.Circles().Empty());
    }

    if (current_shapes & AbstractionShape::Rectangles) {
        CHECK(packed.Rectangles().NumShapes() == 5);
    }
    else
    {
        CHECK(packed.Rectangles().Empty());
    }

    if (current_shapes & AbstractionShape::Triangles) {
        CHECK(packed.Triangles().NumShapes() == 5);
    }
    else
    {
        CHECK(packed.Triangles().Empty());
    }
}

TEST_CASE("Can initialize a completely empty packed collection.")
{
    PackedShapeCollection empty_collection;
    REQUIRE(empty_collection.CollectionSize() == 0);
    REQUIRE(empty_collection.Circles().Empty());
    REQUIRE(empty_collection.Rectangles().Empty());
    REQUIRE(empty_collection.Triangles().Empty());
    REQUIRE_FALSE(empty_collection.Shapes());
}

TEST_CASE("Allow all shape collections in a packed shape collection to be empty.") {
    CircleCollection empty_circles;
    RectangleCollection empty_rectangles;
    TriangleCollection empty_triangles;

    REQUIRE_NOTHROW(PackedShapeCollection(empty_circles, empty_rectangles, empty_triangles));
}

TEST_CASE("Assert when shapes have different sizes in a packed shape collection.") {
    CircleCollection empty_circles;
    RectangleCollection rectangles(5);
    TriangleCollection triangles(8);

    REQUIRE_THROWS_AS(PackedShapeCollection(empty_circles, rectangles, triangles),
                      errors::AbstractionsError);
}

TEST_SUITE_END();
