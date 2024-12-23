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

int main(int nargs, char **args) {
    using abstractions::Prng;
    using abstractions::PrngGenerator;
    using abstractions::UniformDistribution;

    CLI::App app{"Test out the canvas API by drawing some simple images."};
    std::optional<int> seed;
    app.add_option("-s,--seed", seed, "Random seed");

    app.description("Test out the canvas API by drawing some simple images.");
    CLI11_PARSE(app, nargs, args);

    ABSTRACTIONS_INIT_FEATURE_TEST("canvas");

    Prng<> prng{0};
    if (seed) {
        console.Print(fmt::format("Seed: {}", seed.value()));
        prng = Prng(*seed);
    } else {
        prng = Prng(PrngGenerator<>::DrawRandomSeed());
    }

    UniformDistribution uniform_dist(prng);

    try {
        DrawCircles(uniform_dist, test_folder.Path() / "circles.png", 50);
        DrawTriangles(uniform_dist, test_folder.Path() / "triangles.png", 50);
        DrawRectangles(uniform_dist, test_folder.Path() / "rectangles.png", 50);
    } catch (const abstractions::errors::AbstractionsError &) {
        return 1;
    }

    return 0;
}
