#include <abstractions/errors.h>

#include <CLI/CLI.hpp>
#include <indicators/cursor_control.hpp>

#include "generate.h"
#include "render.h"
#include "support.h"

using abstractions::errors::AbstractionsError;

int main(int nargs, char **args) {
    CLI::App app;
    app.description("Create abstract image representations.");
    app.require_subcommand(1);

    GenerateCommand cmd_generate;
    RenderCommand cmd_render;

    Register(app, cmd_generate);
    Register(app, cmd_render);

    indicators::show_console_cursor(false);

    try {
        app.parse(nargs, args);
    } catch (const CLI::ParseError &exc) {
        indicators::show_console_cursor(true);
        return app.exit(exc);
    } catch (const AbstractionsError &exc) {
        indicators::show_console_cursor(true);
        exc.Print();
        return 1;
    }

    indicators::show_console_cursor(true);

    return 0;
}
