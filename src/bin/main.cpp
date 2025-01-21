#include <CLI/CLI.hpp>

#include <abstractions/errors.h>
#include <indicators/cursor_control.hpp>

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

    indicators::show_console_cursor(false);

    try
    {
        app.parse(nargs, args);
    }
    catch (const CLI::ParseError &exc)
    {
        indicators::show_console_cursor(true);
        return app.exit(exc);
    }
    catch (const AbstractionsError &exc)
    {
        indicators::show_console_cursor(true);
        exc.Print();
        return 1;
    }

    indicators::show_console_cursor(true);

    return 0;
}
