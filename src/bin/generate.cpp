#include "generate.h"

#include <abstractions/errors.h>
#include <abstractions/terminal/table.h>

#include <indicators/cursor_control.hpp>
#include <indicators/cursor_movement.hpp>
#include <indicators/progress_bar.hpp>

#include <fmt/format.h>
#include <fmt/std.h>

#include <iostream>
#include <map>

using namespace abstractions;

namespace {

constexpr const char *kEngineOptions = "Abstraction Engine";
constexpr const char *kGeneralOptions = "General Options";
constexpr const char *kPgpeOptions = "PGPE Optimizer";

constexpr const double kDefaultMaxSolutionVelocity = 0.15;
constexpr const double kDefaultImageScale = 1.0;

static cli_helpers::EnumValidator<ImageComparison> ImageComparisonEnum("METRIC",
                                                                       {ImageComparison::L1Norm,
                                                                        ImageComparison::L2Norm});
static cli_helpers::EnumValidator<render::AbstractionShape> AbstractionShapeEnum(
    "SHAPE", {render::AbstractionShape::Circles, render::AbstractionShape::Rectangles,
              render::AbstractionShape::Triangles});

}  // namespace

namespace CLI::detail {

template <>
constexpr const char *type_name<ImageComparison>() {
    return "METRIC";
}

template <>
constexpr const char *type_name<render::AbstractionShape>() {
    return "SHAPE";
}

}  // namespace CLI::detail

CLI::App *GenerateCommand::Init(CLI::App &parent) {
    CLI::callback_t shapes_cb = [this](CLI::results_t results) {
        _config.shapes = Options<render::AbstractionShape>();
        for (const auto &result : results) {
            render::AbstractionShape shape;
            if (!CLI::detail::lexical_cast(result, shape)) {
                return false;
            }
            _config.shapes.Set(shape);
        }

        return true;
    };

    auto app = parent.add_subcommand("generate");
    app->description("Generate the abstract representation of an image.");

    app->add_option("image", _image, "Source image file")->check(CLI::ExistingFile)->required();
    app->add_option("output", _output, "Output path")->required();

    // General options
    app->add_option("--seed", _config.seed, "Set a fixed PRNG seed.")->group(kGeneralOptions);
    app->add_option("--save-intermediate", _per_stage_output, "Optional location where the results at each optimization iteration are stored.")->capture_default_str()->group(kGeneralOptions);
    app->add_option("--scale", _image_scale, "Scale the image by the about before finding the abstract representation.")->capture_default_str()->group(kGeneralOptions);

    // Engine configuration options
    app->add_option("-n,--iterations", _config.iterations,
                    "Maximum number of optimizer iterations.")
        ->capture_default_str()
        ->group(kEngineOptions);
    app->add_option("-k,--samples", _config.num_samples,
                    "Number of samples provided to the PGPE optimizer.")
        ->capture_default_str()
        ->group(kEngineOptions);
    app->add_option("-s,--shapes", _config.num_drawn_shapes,
                    "Number of individual shapes that make up the abstract image.")
        ->capture_default_str()
        ->group(kEngineOptions);

    app->add_option("-t,--shape-type", shapes_cb,
                    "The type of shape to use.  May be repeated to use different shapes.")
        ->transform(AbstractionShapeEnum)
        ->default_str(fmt::format("{}", _config.shapes))
        ->take_all()
        ->group(kEngineOptions);

    app->add_option("--workers", _config.num_workers,
                    "Number of worker threads (default is based on the number of CPU cores).")
        ->capture_default_str()
        ->group(kEngineOptions);

    app->add_option("--metric", _config.comparison_metric,
                    "The comparison metric used when comparing the abstract image to the original.")
        ->transform(ImageComparisonEnum)
        ->default_str(fmt::format("{}", _config.comparison_metric))
        ->group(kEngineOptions);

    // Optimizer configuration options
    _optim_settings.max_speed = kDefaultMaxSolutionVelocity;

    app->add_option("--max-solution-velocity", _optim_settings.max_speed,
                    "Maximum allowable change to the optimizer's solution vector.")
        ->capture_default_str()
        ->group(kPgpeOptions);
    app->add_option("--init-search-radius", _optim_settings.init_search_radius,
                    "Controls how \"far\" the optimizer starts its initial search.")
        ->capture_default_str()
        ->group(kPgpeOptions);
    app->add_option("--momentum", _optim_settings.momentum, "Momentum used in gradient updates.")
        ->capture_default_str()
        ->group(kPgpeOptions);
    app->add_option("--learning-rate", _optim_settings.stddev_learning_rate,
                    "Learning rate for standard deviation estimates.")
        ->capture_default_str()
        ->group(kPgpeOptions);
    app->add_option("--max-stddev-change", _optim_settings.stddev_max_change,
                    "Maximum allowable change to the standard deviation estimate.")
        ->capture_default_str()
        ->group(kPgpeOptions);

    return app;
}

void GenerateCommand::Run() const {
    console.Print("Abstracting {}", _image);
    console.Separator();

    terminal::Table table;
    table
        .AddRow("Shapes", fmt::format("{} [{}]", _config.shapes, _config.num_drawn_shapes))
        .AddRow("Samples", _config.num_samples)
        .AddRow("Scale", _image_scale.value_or(kDefaultImageScale));

    if (_config.seed)
    {
        table.AddRow("Seed", *_config.seed);
    }

    table.AddRow("Iterations", _config.iterations);
    table
        .OuterBorders(false)
        .RowDividers(false)
        .VerticalSeparator("-")
        .Justify(0, terminal::TextJustification::Right)
        .Pad(1);

    table.Render(console);

    console.Separator();

    auto image = Image::Load(_image);
    abstractions_check(image);

    auto engine = Engine::Create(_config, _optim_settings);
    abstractions_check(engine);

    indicators::ProgressBar progbar
    {
        indicators::option::BarWidth{50},
        indicators::option::Start{" ["},
        indicators::option::Fill{"="},
        indicators::option::Lead{">"},
        indicators::option::Remainder{"\u00b7"},
        indicators::option::End{"] "},
        indicators::option::ShowElapsedTime{true},
        indicators::option::ShowRemainingTime{true},
        indicators::option::MaxProgress{_config.iterations}
    };

    if (!_per_stage_output.empty())
    {
        console.Print("Storing optimizer steps to {}", _per_stage_output);
        std::filesystem::remove_all(_per_stage_output);
        std::filesystem::create_directories(_per_stage_output);
    }

    engine->SetCallback([&, this](int i, double cost, ConstRowVectorRef params)
    {
        progbar.set_option(indicators::option::PrefixText{fmt::format("Running Optimizer (Iteration {:>5})", i+1)});
        progbar.tick();

        if (_per_stage_output.empty())
        {
            return;
        }

        if (i % 25 != 0)
        {
            return;
        }

        auto file_name = fmt::format("iter-{:0>5}.png", i);
        auto out_path = _per_stage_output / file_name;
        auto iteration_output = RenderImageAbstraction(image->Width(), image->Height(), _config.shapes, params);
        abstractions_check(iteration_output);
        abstractions_check(iteration_output->Save(out_path));
    });

    auto result = engine->GenerateAbstraction(*image);
    abstractions_check(result);

    indicators::move_up(1);
    indicators::erase_line();

    auto output = RenderImageAbstraction(image->Width(), image->Height(), _config.shapes, result->solution, Pixel(255, 255, 255));
    abstractions_check(output);

    auto output_image_file = _output;
    output_image_file.replace_extension(".png");
    abstractions_check(output->Save(output_image_file));

    console.Print("Finished in {}", result->timing.total_time);
}
