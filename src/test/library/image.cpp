#include <abstractions/errors.h>
#include <abstractions/image.h>
#include <blend2d.h>
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

TEST_CASE("Can compare images.") {
    using abstractions::CompareImagesAbsDiff;
    using abstractions::CompareImagesSquaredDiff;
    using abstractions::Image;

    auto test_image1 = Image::Load(kSamplesPath / "triangles.png");
    auto test_image2 = Image::Load(kSamplesPath / "triangles.png");

    SUBCASE("Comparing the same images should produce a difference of zero.") {
        auto l1_norm = CompareImagesAbsDiff(test_image1, test_image2);
        auto l2_norm = CompareImagesSquaredDiff(test_image1, test_image2);
        REQUIRE(l1_norm);
        REQUIRE(l2_norm);
        REQUIRE(*l1_norm == 0);
        REQUIRE(*l2_norm == 0);
    }

    // Create the "blank image" using the Blend2D API to avoid using the
    // internal 'Canvas' object as its tested separately.
    BLImage buffer(test_image1->Width(), test_image1->Height(), BL_FORMAT_XRGB32);
    {
        BLContext ctx(buffer);
        ctx.clearAll();
        ctx.end();
    }
    Image blank_image(buffer);

    SUBCASE("Comparing against a blank image produces the sum (or squared sum) of the input.") {
        auto pixels = test_image1->Pixels();

        int64_t sum_abs = 0;
        int64_t sum_sq = 0;

        for (int y = 0; y < pixels.Height(); y++) {
            for (int x = 0; x < pixels.Width(); x++) {
                auto pixel = pixels.Get(x, y);
                sum_abs += (pixel.Red() + pixel.Green() + pixel.Blue());
                sum_sq += (pixel.Red() * pixel.Red() + pixel.Green() * pixel.Green() +
                           pixel.Blue() * pixel.Blue());
            }
        }

        int num_pixels = pixels.Width() * pixels.Height();
        double l1_expected = static_cast<double>(sum_abs) / num_pixels;
        double l2_expected = static_cast<double>(sum_sq) / num_pixels;

        auto l1_norm = CompareImagesAbsDiff(test_image1, blank_image);
        auto l2_norm = CompareImagesSquaredDiff(test_image1, blank_image);

        CHECK(*l1_norm == l1_expected);
        CHECK(*l2_norm == l2_expected);
    }
}

TEST_CASE("Errors when attempting to compare images of different sizes.") {
    using abstractions::CompareImagesAbsDiff;
    using abstractions::CompareImagesSquaredDiff;
    using abstractions::Image;

    auto image1 = Image::New(1024, 1024);
    auto image2 = Image::New(512, 512);
    CHECK_FALSE(CompareImagesAbsDiff(image1, image2).has_value());
    CHECK_FALSE(CompareImagesSquaredDiff(image1, image2).has_value());
}

#ifdef ABSTRACTIONS_ENABLE_ASSERTS

TEST_CASE("Errors when provided invalid images.") {
    using abstractions::CompareImagesAbsDiff;
    using abstractions::CompareImagesSquaredDiff;
    using abstractions::Image;
    using abstractions::errors::AbstractionsError;

    auto valid = abstractions::Expected(Image::New(1024, 1024));
    auto invalid = abstractions::errors::report<Image>("Should return an error value.");

    CHECK_THROWS_AS(CompareImagesAbsDiff(valid, invalid), AbstractionsError);
    CHECK_THROWS_AS(CompareImagesAbsDiff(invalid, valid), AbstractionsError);
    CHECK_THROWS_AS(CompareImagesSquaredDiff(valid, invalid), AbstractionsError);
    CHECK_THROWS_AS(CompareImagesSquaredDiff(invalid, valid), AbstractionsError);
}

#endif

TEST_SUITE_END();
