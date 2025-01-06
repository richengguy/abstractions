#include <CLI/CLI.hpp>

#include "support.h"
#include "render.h"
#include "find.h"

int main(int nargs, char **args) {
    CLI::App app;
    app.description("Generate abstract images.");
    app.require_subcommand(1);

    FindCommand cmd_find;
    RenderCommand cmd_render;

    Register(app, cmd_find);
    Register(app, cmd_render);

    CLI11_PARSE(app, nargs, args);

    return 0;
}
