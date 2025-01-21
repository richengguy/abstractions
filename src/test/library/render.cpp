#include <abstractions/errors.h>
#include <abstractions/image.h>
#include <abstractions/math/random.h>
#include <abstractions/render/canvas.h>
#include <doctest/doctest.h>

using namespace abstractions;

TEST_SUITE_BEGIN("render");

#ifdef ABSTRACTIONS_ENABLE_ASSERTS

TEST_CASE("Assertion occurs when expected to Canvas constructor is empty.") {
    auto bad_image = abstractions::errors::report<Image>("Should see this assert.");
    REQUIRE_THROWS_AS(render::Canvas(bad_image, {}), errors::AbstractionsError);
    REQUIRE_THROWS_AS(render::Canvas(bad_image, Prng<>{123}), errors::AbstractionsError);
}

#endif

TEST_SUITE_END();
