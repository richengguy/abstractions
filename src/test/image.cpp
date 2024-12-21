#include <abstractions/image.h>
#include <doctest/doctest.h>

#include <array>
#include <filesystem>
#include <string>

#include "samples-path.h"

TEST_SUITE_BEGIN("image");

TEST_CASE("Errors reported correctly when loading images.") {
    using abstractions::Image;

    auto image = Image::Load("unknown.jpg");

    INFO(image.error().value_or("No error"));
    REQUIRE_FALSE(image.has_value());
    REQUIRE(image.error().has_value());
}

TEST_CASE("Can load supported image formats.") {
    using abstractions::Image;

    const std::array<std::filesystem::path, 2> kFiles{
        kSamplesPath / "triangles.jpg",
        kSamplesPath / "triangles.png",
    };

    for (auto &file : kFiles) {
        auto image = Image::Load(file);

        INFO(image.error().value_or("Successfully loaded image."));
        REQUIRE(image);
        REQUIRE(image->Width() == 256);
        REQUIRE(image->Height() == 512);
    }
}

TEST_CASE("Can pack and unpack pixels into a 32-bit integer.") {
    using abstractions::Pixel;

    const uint8_t red = 0xA0;
    const uint8_t green = 0xB1;
    const uint8_t blue = 0xC2;
    const uint8_t alpha = 0xD3;

    Pixel pixel(red, green, blue, alpha);

    REQUIRE(pixel == 0xD3A0B1C2);
    REQUIRE(pixel.Red() == red);
    REQUIRE(pixel.Green() == green);
    REQUIRE(pixel.Blue() == blue);
    REQUIRE(pixel.Alpha() == alpha);
}

TEST_SUITE_END();
