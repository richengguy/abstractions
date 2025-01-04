#include "abstractions/engine.h"

#include <abstractions/profile.h>
#include <abstractions/threads/threadpool.h>
#include <abstractions/render/renderer.h>

#include <functional>
#include <vector>

namespace abstractions
{

namespace
{

/// @brief Contains the optimizer along with everything it needs to perform
///     the "sample" and "optimize" operations.
struct OptimizerPayload
{
    std::reference_wrapper<PgpeOptimizer> optimizer;
    MatrixRef samples;
    ColumnVectorRef costs;
};

/// @brief Contains everything needed to render a single image and compute the
///     matching cost.
struct RenderPayload
{
    std::reference_wrapper<const Image> reference;
    std::vector<render::Renderer> renderers;
    MatrixRef samples;
    ColumnVectorRef costs;
    const ImageComparison comparison_metric;
};

/// @brief Generate the set of samples needed for estimating sample costs.
struct GenerateSolutionSamples : public threads::IJobFunction
{
    Error operator()(threads::JobContext &ctx) const override
    {
        auto payload = ctx.Data<OptimizerPayload>();
        if (!payload.has_value())
        {
            return payload.error();
        }

        auto &optim = payload->optimizer.get();
        return optim.Sample(payload->samples);
    }
};

/// @brief Run the PGPE optimizer to update the internal solution.
struct RunOptimizer : public threads::IJobFunction
{
    Error operator()(threads::JobContext &ctx) const override
    {
        auto payload = ctx.Data<OptimizerPayload>();
        if (!payload.has_value())
        {
            return payload.error();
        }

        auto &optim = payload->optimizer.get();
        return optim.Update(payload->samples, payload->costs);
    }
};

/// @brief Render the set of images from the PGPE optimizer samples and compute
///     the per-sample costs.
struct RenderAndCompare : public threads::IJobFunction
{
    Error operator()(threads::JobContext &ctx) const override
    {
        auto payload = ctx.Data<RenderPayload>();
        if (!payload.has_value())
        {
            return payload.error();
        }

        // TODO: Figure this out

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

    Matrix samples;
    ColumnVector costs;

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

        samples = Matrix::Zero(_config.num_samples, image_abstraction.TotalDimensions());
        costs = ColumnVector::Zero(_config.num_samples);
    }

    // Setup the thread payloads.
    OptimizerPayload optim_payload{
        .optimizer = *optimizer,
        .samples = samples,
        .costs = costs
    };

    RenderPayload render_payload{
        .reference = reference,
        .renderers = std::vector<render::Renderer>(),
        .samples = samples,
        .costs = costs,
        .comparison_metric = _config.comparison_metric
    };

    // TODO: init the renderers

    // Now run the "sample->render->optimize" loop, keeping track of how the
    // solution is performing.
    OperationTiming pgpe_timing;
    OperationTiming render_timing;
    OperationTiming sampling_timing;

    for (int i = 0; i < _config.max_iterations; i++)
    {
        // Run the sampling step
        auto sample_job = thread_pool.SubmitWithPayload<GenerateSolutionSamples>(0, optim_payload);
        auto sample_result = sample_job.get();

        if (sample_result.error) {
            return errors::report<OptimizationResult>(sample_result.error);
        }

        sampling_timing.AddSample(sample_result.time);

        // Render images from the generated samples and compute the costs.
        std::vector<threads::Job::Future> futures;
        for (int j = 0; j < _config.num_samples; j++)
        {
            auto render_job = thread_pool.SubmitWithPayload<RenderAndCompare>(j, render_payload);
            // TODO: figure out how to wait on these in a reasonable way
        }

        // Run the optimizer and update its state.
        auto update_job = thread_pool.SubmitWithPayload<GenerateSolutionSamples>(0, optim_payload);
        auto update_result = update_job.get();

        if (update_result.error) {
            return errors::report<OptimizationResult>(update_result.error);
        }

        pgpe_timing.AddSample(update_result.time);

        // Invoke any callbacks
        // TODO
    }

    // Wrap things up
    auto end_to_end_time = e2e_timer.GetElapsedTime();
}

}
