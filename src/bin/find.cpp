#include "find.h"

#include <abstractions/errors.h>

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

CLI::App *FindCommand::Init(CLI::App &parent) {
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

    auto app = parent.add_subcommand("find");
    app->description("Find the abstract representation of an image.");

    app->add_option("image", _image, "Source image file")->check(CLI::ExistingFile)->required();
    app->add_option("output", _output, "Output path")->required();

    // General options
    app->add_option("--seed", _config.seed, "Set a fixed PRNG seed.")->group(kGeneralOptions);
    app->add_option("--save-intermediate", _per_stage_output, "Optional location where the results at each optimization iteration are stored.")->capture_default_str()->group(kGeneralOptions);
    app->add_option("--scale", _image_scale, "Scale the image by the about before finding the abstract representation.")->capture_default_str()->group(kGeneralOptions);

    // Engine configuration options
    app->add_option("-n,--iterations", _config.max_iterations,
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

    app->add_flag("--costs-ranking", _config.costs_ranking,
                  "Maps all costs onto [-0.5, 0.5] to mitigate against outliers.")
        ->capture_default_str()
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

void FindCommand::Run() const {
    console.Print("Abstracting {}", _image);
    console.Separator();
    console.Print("         Shapes  - {} [{}]", _config.shapes, _config.num_drawn_shapes);
    console.Print("         Samples - {}", _config.num_samples);
    console.Print("         Scale   - {}", _image_scale.value_or(kDefaultImageScale));
    console.Print("  Max Iterations - {}", _config.max_iterations);
    console.Separator();

    auto image = Image::Load(_image);
    abstractions_check(image);

    auto engine = Engine::Create(_config, _optim_settings);
    abstractions_check(engine);

    auto result = engine->GenerateAbstraction(*image);
    abstractions_check(result);

    console.Print("Finished in {}", result->timing.total_time);
}
