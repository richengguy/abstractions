#include <abstractions/image.h>
#include <doctest/doctest.h>

#include <array>
#include <filesystem>
#include <string>

#include "support.h"

TEST_SUITE_BEGIN("image");

TEST_CASE("Errors reported correctly when loading images.") {
    using abstractions::Image;

    auto image = Image::Load("unknown.jpg");

    INFO(image.error().value_or("No error"));
    REQUIRE_FALSE(image.has_value());
    REQUIRE(image.error().has_value());
}

TEST_CASE("Error reported when trying to save an invalid image type.") {
    using abstractions::Image;

    auto image = Image::Load(kSamplesPath / "triangles.jpg");
    auto err = image->Save("invalid-type.txt");
    INFO(err.value_or("No error"));
    REQUIRE(err);
}

TEST_CASE("Error reported when creating a new image with invalid dimensions.") {
    using abstractions::Image;

    SUBCASE("Error with invalid width.") {
        auto image = Image::New(-1, 128);
        INFO(image.error().value_or("No error"));
        REQUIRE_FALSE(image.has_value());
        REQUIRE(image.error().has_value());
    }

    SUBCASE("Error with invalid height.") {
        auto image = Image::New(128, -1);
        INFO(image.error().value_or("No error"));
        REQUIRE_FALSE(image.has_value());
        REQUIRE(image.error().has_value());
    }
}

TEST_CASE("Image bit-depth is 24bpp for RGB and 32bpp for ARGB.") {
    using abstractions::Image;
    using abstractions::PixelFormat;

    SUBCASE("RGB images have 32bpp but 'RGB' format.") {
        auto image = Image::New(128, 128);
        INFO("Bit depth: ", image->Depth());
        REQUIRE(image->Depth() == 32);
        REQUIRE(image->Format() == PixelFormat::RGB);
    }

    SUBCASE("ARGB images have 32bpp with 'RGBWithPremultAlpha' format.") {
        auto image = Image::New(128, 128, true);
        INFO("Bit depth: ", image->Depth());
        REQUIRE(image->Depth() == 32);
        REQUIRE(image->Format() == PixelFormat::RGBWithPremultAlpha);
    }
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

TEST_CASE("Can save an image.") {
    using abstractions::Image;
    using abstractions::tests::TempFolder;

    TempFolder temp_folder;

    auto test_file = temp_folder.Path() / "test.png";
    INFO("Test File: ", test_file);

    auto new_image = Image::New(1024, 728);
    REQUIRE(new_image);

    auto err = new_image->Save(test_file);
    if (err) {
        FAIL("Had error: ", err.value());
    }

    REQUIRE(std::filesystem::exists(test_file));

    auto loaded_image = Image::Load(test_file);
    REQUIRE(loaded_image);
    REQUIRE(loaded_image->Width() == 1024);
    REQUIRE(loaded_image->Height() == 728);
}

TEST_SUITE_END();
