#include "abstractions/engine.h"

#include <abstractions/profile.h>
#include <abstractions/threads/threadpool.h>

namespace abstractions
{

namespace
{

/// @brief Generate the set of samples needed for estimating sample costs.
struct GenerateSolutionSamples : public threads::IJobFunction
{
    Error operator()(threads::JobContext &ctx) const override
    {
        return errors::no_error;
    }
};

/// @brief Run the PGPE optimizer to update the internal solution.
struct RunOptimizer : public threads::IJobFunction
{
    Error operator()(threads::JobContext &ctx) const override
    {
        return errors::no_error;
    }
};

/// @brief Render the set of images from the PGPE optimizer samples and compute
///     the per-sample costs.
struct RenderAndCompare : public threads::IJobFunction
{
    Error operator()(threads::JobContext &ctx) const override
    {
        return errors::no_error;
    }
};

}

Error EngineConfig::Validate() const
{
    if (max_iterations < 1)
    {
        return "Maximum number of iterations cannot be negative.";
    }

    if (num_samples < 1 || num_samples % 2 != 0)
    {
        return "The number of samples must be greater than zero and an even number.";
    }

    if (num_drawn_shapes < 1)
    {
        return "The number of drawn shapes must be greater than zero.";
    }

    if (num_workers && num_workers < 1)
    {
        return "The number of thread workers must be greater than zero.";
    }

    return errors::no_error;
}

Expected<Engine> Engine::Create(const EngineConfig &config, const PgpeOptimizerSettings &optim_settings)
{
    if (auto err = config.Validate())
    {
        return errors::report<Engine>(err);
    }

    if (auto err = optim_settings.Validate())
    {
        return errors::report<Engine>(err);
    }

    return Engine(config, optim_settings);
}

Engine::Engine(const EngineConfig &config, const PgpeOptimizerSettings &optim)
    : _config{config},
      _optim_settings{optim}
{

}

Expected<OptimizationResult> Engine::GenerateAbstraction(const Image &reference) const
{
    const int width = reference.Width();
    const int height = reference.Height();

    Timer e2e_timer;

    // The bulk of the work is done on the thread pool.  It is set up to have
    // enough resources for processing all sample rendering requests, along with
    // some buffer.  The number of workers, unless overridden in the engine
    // config, will depend on the number of available CPU cores.
    threads::ThreadPoolConfig multithreading_config{
        .num_workers = _config.num_workers,
        .queue_depth = _config.num_samples + 1,
    };
    threads::ThreadPool thread_pool(multithreading_config);

    // Create the generator for all PRNGs that will be used during the
    // optimization.  This wil use either a pre-configured seed or a randomly
    // chosen one.
    PrngGenerator<> prng_generator(_config.seed);

    // First, create the optimizer.  Use a generated PRNG to get the seed.
    auto pgpe_prng = prng_generator.CreatePrng();
    auto optimizer = PgpeOptimizer::New(_optim_settings);
    if (!optimizer.has_value())
    {
        return errors::report<OptimizationResult>(optimizer.error());
    }
    optimizer->SetPrngSeed(pgpe_prng.seed());

    // Do the initial abstract shape generation to prime the optimizer with an
    // initial solution.
    render::PackedShapeCollection image_abstraction;
    OperationTiming init_timing;
    {
        Profile profiler{init_timing};
        render::ShapeGenerator shape_generator(width, height, prng_generator.CreatePrng());

        render::CircleCollection circles;
        render::RectangleCollection rectangles;
        render::TriangleCollection triangles;

        if (_config.shapes & render::AbstractionShape::Circles)
        {
            circles = shape_generator.RandomCircles(_config.num_drawn_shapes);
        }

        if (_config.shapes & render::AbstractionShape::Rectangles)
        {
            rectangles = shape_generator.RandomRectangles(_config.num_drawn_shapes);
        }

        if (_config.shapes & render::AbstractionShape::Triangles)
        {
            triangles = shape_generator.RandomTriangles(_config.num_drawn_shapes);
        }

        image_abstraction = render::PackedShapeCollection(circles, rectangles, triangles);
        optimizer->Initialize(image_abstraction.AsPackedVector());
    }

    // Now run the "sample->render->optimize" loop, keeping track of how the
    // solution is performing.

    for (int i = 0; i < _config.max_iterations; i++)
    {

    }

    // Wrap things up
    auto end_to_end_time = e2e_timer.GetElapsedTime();
}

}
