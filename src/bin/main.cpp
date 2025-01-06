#include <CLI/CLI.hpp>

#include <abstractions/errors.h>

#include "find.h"
#include "render.h"
#include "support.h"

using abstractions::errors::AbstractionsError;

int main(int nargs, char **args) {
    CLI::App app;
    app.description("Generate abstract images.");
    app.require_subcommand(1);

    FindCommand cmd_find;
    RenderCommand cmd_render;

    Register(app, cmd_find);
    Register(app, cmd_render);

    try
    {
        app.parse(nargs, args);
    }
    catch (const CLI::ParseError &exc)
    {
        return app.exit(exc);
    }
    catch (const AbstractionsError &exc)
    {
        exc.Print();
        return 1;
    }

    return 0;
}
