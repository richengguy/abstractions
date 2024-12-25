#include <abstractions/errors.h>
#include <abstractions/image.h>
#include <abstractions/math/random.h>
#include <abstractions/render/canvas.h>
#include <doctest/doctest.h>

TEST_SUITE_BEGIN("render");

TEST_CASE("Assertion occurs when expected to Canvas constructor is empty.") {
    using abstractions::Canvas;
    using abstractions::Expected;
    using abstractions::Image;
    using abstractions::errors::AbstractionsError;

    auto bad_image = abstractions::errors::report<Image>("Should see this assert.");
    REQUIRE_THROWS_AS(Canvas(bad_image, {}), AbstractionsError);
    REQUIRE_THROWS_AS(Canvas(bad_image, abstractions::Prng<>{123}), AbstractionsError);
}

TEST_SUITE_END();
