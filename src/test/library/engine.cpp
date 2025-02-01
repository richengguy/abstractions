#include <abstractions/engine.h>
#include <abstractions/render/shapes.h>
#include <doctest/doctest.h>

#include <abstractions/math/types.h>

#include "support.h"

using namespace abstractions;

TEST_CASE("Can serialize/deserialize OptimizationResult.")
{
    tests::TempFolder temp_folder;

    RowVector solution = RowVector::LinSpaced(14, 1, 14);

    OptimizationResult result
    {
        .solution = solution,
        .cost = 123,
        .iterations = 456,
        .aspect_ratio = 2.5,
        .shapes = render::AbstractionShape::Triangles | render::AbstractionShape::Circles,
        .seed = 789,
        .timing = TimingReport(0, 0),
    };

    result.Save(temp_folder.Path() / "test.json");
    auto restored = OptimizationResult::Load(temp_folder.Path() / "test.json");

    REQUIRE(restored.has_value());
    CHECK(result.solution == restored->solution);
    CHECK(result.cost == restored->cost);
    CHECK(result.iterations == restored->iterations);
    CHECK(result.aspect_ratio == restored->aspect_ratio);
    CHECK(result.shapes == restored->shapes);
    CHECK(result.seed == restored->seed);
}
