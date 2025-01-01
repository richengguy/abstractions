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

using namespace abstractions;

namespace {

void DrawCircles(render::ShapeGenerator &generator, const std::filesystem::path &output,
                 const int num_circles) {
    auto surface = Image::New(kWidth, kHeight);
    abstractions_check(surface);
    {
        render::Canvas canvas{surface};
        canvas.Clear();

        auto circles = generator.RandomCircles(num_circles);
        abstractions_check(canvas.DrawFilledCircles(circles.Params));
    }
    surface->Save(output);
}

void DrawTriangles(render::ShapeGenerator &generator, const std::filesystem::path &output,
                   const int num_triangles) {
    auto surface = Image::New(kWidth, kHeight);
    abstractions_check(surface);
    {
        render::Canvas canvas{surface};
        canvas.Clear();

        auto triangles = generator.RandomTriangles(num_triangles);
        abstractions_check(canvas.DrawFilledTriangles(triangles.Params));
    }
    surface->Save(output);
}

void DrawRectangles(render::ShapeGenerator &generator, const std::filesystem::path &output,
                    const int num_rects) {
    auto surface = Image::New(kWidth, kHeight);
    abstractions_check(surface);
    {
        render::Canvas canvas{surface};
        canvas.Clear();

        auto rects = generator.RandomRectangles(num_rects);
        abstractions_check(canvas.DrawFilledRectangles(rects.Params));
    }
    surface->Save(output);
}

void RandomFillCanvas(const std::filesystem::path &output) {
    auto surface = Image::New(kWidth, kHeight);
    abstractions_check(surface);
    {
        render::Canvas canvas{surface};
        canvas.RandomFill();
    }
    surface->Save(output);
}

}  // namespace

ABSTRACTIONS_FEATURE_TEST() {
    render::ShapeGenerator generator(kWidth, kHeight, prng);

    console.Print("Draw circles.");
    DrawCircles(generator, output_folder.FilePath("circles.png"), 50);

    console.Print("Draw triangles.");
    DrawTriangles(generator, output_folder.FilePath("triangles.png"), 50);

    console.Print("Draw rectangles.");
    DrawRectangles(generator, output_folder.FilePath("rectangles.png"), 50);

    console.Print("Fill with random values.");
    RandomFillCanvas(output_folder.FilePath("random.png"));
}

ABSTRACTIONS_FEATURE_TEST_MAIN("canvas", "Test out the canvas API by drawing some simple images");
