#include <abstractions/errors.h>
#include <abstractions/image.h>
#include <abstractions/math/matrices.h>
#include <abstractions/render/canvas.h>
#include <abstractions/render/shapes.h>
#include <fmt/format.h>

#include <CLI/CLI.hpp>
#include <string>

#include "support.h"

constexpr int kWidth = 640;
constexpr int kHeight = 480;

namespace {

void DrawCircles(abstractions::ShapeGenerator &generator, const std::filesystem::path &output,
                 const int num_circles) {
    using abstractions::Canvas;
    using abstractions::Image;
    using abstractions::Matrix;

    auto surface = Image::New(kWidth, kHeight);
    abstractions_check(surface);
    {
        Canvas canvas{*surface};
        canvas.Clear();

        auto circles = generator.RandomCircles(num_circles);
        abstractions_check(canvas.DrawFilledCircles(circles.Params));
    }
    surface->Save(output);
}

void DrawTriangles(abstractions::ShapeGenerator &generator, const std::filesystem::path &output,
                   const int num_triangles) {
    using abstractions::Canvas;
    using abstractions::Image;
    using abstractions::Matrix;

    auto surface = Image::New(kWidth, kHeight);
    abstractions_check(surface);
    {
        Canvas canvas{*surface};
        canvas.Clear();

        auto triangles = generator.RandomTriangles(num_triangles);
        abstractions_check(canvas.DrawFilledTriangles(triangles.Params));
    }
    surface->Save(output);
}

void DrawRectangles(abstractions::ShapeGenerator &generator, const std::filesystem::path &output,
                    const int num_rects) {
    using abstractions::Canvas;
    using abstractions::Image;
    using abstractions::Matrix;

    auto surface = Image::New(kWidth, kHeight);
    abstractions_check(surface);
    {
        Canvas canvas{*surface};
        canvas.Clear();

        auto rects = generator.RandomRectangles(num_rects);
        abstractions_check(canvas.DrawFilledRectangles(rects.Params));
    }
    surface->Save(output);
}

}  // namespace

ABSTRACTIONS_FEATURE_TEST() {
    abstractions::ShapeGenerator generator(kWidth, kHeight, prng);

    console.Print("Draw circles.");
    DrawCircles(generator, output_folder.FilePath("circles.png"), 50);

    console.Print("Draw triangles.");
    DrawTriangles(generator, output_folder.FilePath("triangles.png"), 50);

    console.Print("Draw rectangles.");
    DrawRectangles(generator, output_folder.FilePath("rectangles.png"), 50);
}

ABSTRACTIONS_FEATURE_TEST_MAIN("canvas", "Test out the canvas API by drawing some simple images");
