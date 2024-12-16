#include <abstractions/image.h>

#include <doctest/doctest.h>

TEST_SUITE_BEGIN("image");

TEST_CASE("Errors reported correctly when loading images.")
{
    using abstractions::Image;

    auto image = Image::Load("unknown.jpg");

    INFO(image.error().value_or("No error"));
    REQUIRE_FALSE(image.has_value());
    REQUIRE(image.error().has_value());
}

TEST_CASE("Can pack and unpack pixels into a 32-bit integer.")
{
    INFO("Implement this!");
    REQUIRE(false);
}

TEST_SUITE_END();
