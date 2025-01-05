#include "render.h"

#include <fmt/format.h>

CLI::App *RenderCommand::Init(CLI::App &parent)
{
    auto cmd = parent.add_subcommand("render", "Render an existing abstract image.");
    return cmd;
}

void RenderCommand::Run() const
{
    fmt::println("Not currently implemented.");
}
