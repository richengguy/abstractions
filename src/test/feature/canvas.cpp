#include <CLI/CLI.hpp>
#include <fmt/format.h>

#include <abstractions/image.h>
#include <abstractions/render.h>
#include <abstractions/errors.h>
#include <abstractions/math/matrices.h>

#include <string>

#include "support.h"

void DrawCircles(const std::filesystem::path &output, const int num_circles)
{
    using abstractions::Canvas;
    using abstractions::Image;
    using abstractions::Matrix;

    auto surface = Image::New(640, 480);
    abstractions_assert(surface.has_value());
}

int main(int nargs, char **args)
{
    CLI::App app{"Test out the canvas API by drawing some simple images."};
    int seed_value = -1;
    auto seed = app.add_option("-s,--seed", seed_value, "Random seed");

    app.description("Test out the canvas API by drawing some simple images.");
    CLI11_PARSE(app, nargs, args);

    ABSTRACTIONS_INIT_FEATURE_TEST("canvas");

    if (*seed)
    {
        console.Print(fmt::format("Seed: {}", seed_value));
    }

    return 0;
}
