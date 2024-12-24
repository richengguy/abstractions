#include <abstractions/errors.h>
#include <abstractions/image.h>
#include <abstractions/math/matrices.h>
#include <abstractions/render.h>
#include <fmt/format.h>

#include <CLI/CLI.hpp>
#include <string>

#include "support.h"

void DrawCircles(abstractions::UniformDistribution<> &dist, const std::filesystem::path &output,
                 const int num_circles) {
    using abstractions::Canvas;
    using abstractions::Image;
    using abstractions::Matrix;

    auto surface = Image::New(640, 480);
    abstractions_check(surface);
    {
        Canvas canvas{*surface};
        canvas.Clear();

        Matrix params = abstractions::RandomMatrix(num_circles, 7, dist);
        abstractions_check(canvas.DrawFilledCircles(params));
    }
    surface->Save(output);
}

void DrawTriangles(abstractions::UniformDistribution<> &dist, const std::filesystem::path &output,
                   const int num_triangles) {
    using abstractions::Canvas;
    using abstractions::Image;
    using abstractions::Matrix;

    auto surface = Image::New(640, 480);
    abstractions_check(surface);
    {
        Canvas canvas{*surface};
        canvas.Clear();

        Matrix params = abstractions::RandomMatrix(num_triangles, 10, dist);
        abstractions_check(canvas.DrawFilledTriangles(params));
    }
    surface->Save(output);
}

void DrawRectangles(abstractions::UniformDistribution<> &dist, const std::filesystem::path &output,
                    const int num_rects) {
    using abstractions::Canvas;
    using abstractions::Image;
    using abstractions::Matrix;

    auto surface = Image::New(640, 480);
    abstractions_check(surface);
    {
        Canvas canvas{*surface};
        canvas.Clear();

        Matrix params = abstractions::RandomMatrix(num_rects, 8, dist);
        abstractions_check(canvas.DrawFilledRectangles(params));
    }
    surface->Save(output);
}

ABSTRACTIONS_FEATURE_TEST() {
    abstractions::UniformDistribution uniform_dist(prng);

    console.Print("Draw circles.");
    DrawCircles(uniform_dist, output_folder.FilePath("circles.png"), 50);

    console.Print("Draw triangles.");
    DrawTriangles(uniform_dist, output_folder.FilePath("triangles.png"), 50);

    console.Print("Draw rectangles.");
    DrawRectangles(uniform_dist, output_folder.FilePath("rectangles.png"), 50);
}

ABSTRACTIONS_FEATURE_TEST_MAIN("canvas", "Test out the canvas API by drawing some simple images");
