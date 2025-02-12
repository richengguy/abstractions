#include "render.h"

#include <abstractions/engine.h>
#include <abstractions/terminal/table.h>
#include <fmt/color.h>
#include <fmt/format.h>

using namespace abstractions;

CLI::App *RenderCommand::Init(CLI::App &parent) {
    auto cmd = parent.add_subcommand("render", "Render an existing abstract image.");

    auto select_dim = cmd->add_option_group("Image Dimension");
    select_dim->add_flag("--width", _use_width, "output image width");
    select_dim->add_flag("--height", _use_height, "output image height");
    select_dim->require_option(1);

    cmd->add_option("size", _dim, "output dimension size");
    cmd->add_option("json", _json, "image abstraction JSON file")->transform(CLI::ExistingFile);
    cmd->add_option("output", _output, "output image file");

    return cmd;
}

void RenderCommand::Run() const {
    // Sanity check that either _use_width or _use_height is set, but not both.
    abstractions_assert(_use_width || _use_height);
    abstractions_assert(_use_width && !_use_height || _use_height && !_use_width);

    console.Print("Rendering {}", _json);

    if (_dim < 32) {
        console.Print("{} - Output size must be at least '32' pixels.",
                      fmt::styled("Error", fmt::emphasis::italic | fmt::fg(fmt::color::red)));
        return;
    }

    auto abstraction = OptimizationResult::Load(_json);
    abstractions_check(abstraction);

    int width = 0;
    int height = 0;

    if (_use_width) {
        width = _dim;
        height = std::round(_dim / abstraction->aspect_ratio);
    }

    if (_use_height) {
        width = std::round(abstraction->aspect_ratio * _dim);
        height = _dim;
    }

    terminal::Table config;
    config.AddRow("Width", width)
        .AddRow("Height", height)
        .AddRow("Output", _output)
        .VerticalSeparator("-")
        .OuterBorders(false)
        .RowDividers(false)
        .Justify(0, terminal::TextJustification::Right)
        .Pad(1)
        .Render(console);

    auto image = RenderImageAbstraction(width, height, abstraction->shapes, abstraction->solution,
                                        1.0,
                                        Pixel(255, 255, 255));
    abstractions_check(image);
    abstractions_check(image->Save(_output));
}
