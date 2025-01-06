#include "find.h"

#include <fmt/format.h>
#include <fmt/std.h>

CLI::App *FindCommand::Init(CLI::App &parent)
{
    auto app = parent.add_subcommand("find");
    app->description("Find the abstract representation of an image.");

    app->add_option("--seed,-s", _seed, "PRNG seed");
    app->add_option("image", _image, "Source image file")->required();

    return app;
}

void FindCommand::Run() const
{
    fmt::println("Image File: {}", _image);
    fmt::println("Do something!!!");
}
