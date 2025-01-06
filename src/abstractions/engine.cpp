#include "abstractions/engine.h"

#include <abstractions/profile.h>
#include <abstractions/render/renderer.h>
#include <abstractions/threads/threadpool.h>

#include <functional>
#include <vector>

namespace abstractions {

namespace {

/// @brief Compute the comparison costs.
/// @param metric comparison metric
/// @param ref reference image
/// @param tgt target image (what's being compared to the reference)
/// @return the cost, or an error if something went wrong
Expected<double> ComputeCost(ImageComparison metric, const Image &ref, const Image &tgt) {
    switch (metric) {
        case ImageComparison::L1Norm:
            return CompareImagesAbsDiff(ref, tgt);
        case ImageComparison::L2Norm:
            return CompareImagesSquaredDiff(ref, tgt);
    }

    return errors::report<double>("Unknown comparison metric.");
}

/// @brief Helper to convert an "internal" data type to an "external" one.
/// @param op operations timing
/// @return the process time
TimingReport::ProcessTime FromOperationTiming(const OperationTiming &op) {
    auto timing = op.GetTiming();
    return TimingReport::ProcessTime{
        .total = timing.total,
        .mean = timing.mean,
        .standard_deviation = timing.stddev,
    };
}

/// @brief Contains the optimizer along with everything it needs to perform
///     the "sample" and "optimize" operations.
struct OptimizerPayload {
    std::reference_wrapper<PgpeOptimizer> optimizer;
    MatrixRef samples;
    ColumnVectorRef costs;
};

/// @brief Contains everything needed to render a single image and compute the
///     matching cost.
struct RenderPayload {
    std::reference_wrapper<const Image> reference;
    std::vector<render::Renderer> renderers;
    MatrixRef samples;
    ColumnVectorRef costs;
    const Options<render::AbstractionShape> shapes;
    const ImageComparison comparison_metric;
};

/// @brief Generate the set of samples needed for estimating sample costs.
struct GenerateSolutionSamples : public threads::IJobFunction {
    Error operator()(threads::JobContext &ctx) const override {
        auto payload = ctx.Data<OptimizerPayload>();
        if (!payload.has_value()) {
            return payload.error();
        }

        auto &optim = payload->optimizer.get();
        return optim.Sample(payload->samples);
    }
};

/// @brief Run the PGPE optimizer to update the internal solution.
struct RunOptimizer : public threads::IJobFunction {
    Error operator()(threads::JobContext &ctx) const override {
        auto payload = ctx.Data<OptimizerPayload>();
        if (!payload.has_value()) {
            return payload.error();
        }

        auto &optim = payload->optimizer.get();
        return optim.Update(payload->samples, payload->costs);
    }
};

/// @brief Render the set of images from the PGPE optimizer samples and compute
///     the per-sample costs.
struct RenderAndCompare : public threads::IJobFunction {
    Error operator()(threads::JobContext &ctx) const override {
        auto payload = ctx.Data<RenderPayload>();
        if (!payload.has_value()) {
            return payload.error();
        }

        render::PackedShapeCollection sampled_shapes(payload->shapes,
                                                     payload->samples.row(ctx.Index()));

        // Render the test image, using a random background to avoid biasing
        // blank areas.
        auto &renderer = payload->renderers.at(ctx.Index());
        renderer.UseRandomBackgroundFill(true);
        renderer.Render(sampled_shapes);

        // Compute the matching cost of the rendered image with the reference.
        auto cost =
            ComputeCost(payload->comparison_metric, payload->reference, renderer.DrawingSurface());

        if (!cost.has_value()) {
            return cost.error();
        }

        // NOTE: Storing the *negative* costs because PGPE finds a maximum, not
        // a minimum.
        payload->costs(ctx.Index()) = -(*cost);

        return errors::no_error;
    }
};

}  // namespace

Error EngineConfig::Validate() const {
    if (max_iterations < 1) {
        return "Maximum number of iterations cannot be negative.";
    }

    if (num_samples < 1 || num_samples % 2 != 0) {
        return "The number of samples must be greater than zero and an even number.";
    }

    if (num_drawn_shapes < 1) {
        return "The number of drawn shapes must be greater than zero.";
    }

    if (num_workers && num_workers < 1) {
        return "The number of thread workers must be greater than zero.";
    }

    return errors::no_error;
}

Expected<Engine> Engine::Create(const EngineConfig &config,
                                const PgpeOptimizerSettings &optim_settings) {
    if (auto err = config.Validate()) {
        return errors::report<Engine>(err);
    }

    if (auto err = optim_settings.Validate()) {
        return errors::report<Engine>(err);
    }

    return Engine(config, optim_settings);
}

Engine::Engine(const EngineConfig &config, const PgpeOptimizerSettings &optim) :
    _config{config},
    _optim_settings{optim} {}

Expected<OptimizationResult> Engine::GenerateAbstraction(const Image &reference) const {
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
    if (!optimizer.has_value()) {
        return errors::report<OptimizationResult>(optimizer.error());
    }
    optimizer->SetPrngSeed(pgpe_prng.seed());

    // Do the initial abstract shape generation to prime the optimizer with an
    // initial solution.
    Matrix samples;
    ColumnVector costs;

    OperationTiming init_timing;
    {
        Profile profiler{init_timing};
        render::ShapeGenerator shape_generator(width, height, prng_generator.CreatePrng());

        render::CircleCollection circles;
        render::RectangleCollection rectangles;
        render::TriangleCollection triangles;

        if (_config.shapes & render::AbstractionShape::Circles) {
            circles = shape_generator.RandomCircles(_config.num_drawn_shapes);
        }

        if (_config.shapes & render::AbstractionShape::Rectangles) {
            rectangles = shape_generator.RandomRectangles(_config.num_drawn_shapes);
        }

        if (_config.shapes & render::AbstractionShape::Triangles) {
            triangles = shape_generator.RandomTriangles(_config.num_drawn_shapes);
        }

        render::PackedShapeCollection init_shapes(circles, rectangles, triangles);
        optimizer->Initialize(init_shapes.AsPackedVector());

        samples = Matrix::Zero(_config.num_samples, init_shapes.TotalDimensions());
        costs = ColumnVector::Zero(_config.num_samples);
    }

    // Setup the thread payloads.
    OptimizerPayload optim_payload{
        .optimizer = *optimizer,
        .samples = samples,
        .costs = costs,
    };

    RenderPayload render_payload{
        .reference = reference,
        .renderers = std::vector<render::Renderer>(),
        .samples = samples,
        .costs = costs,
        .shapes = _config.shapes,
        .comparison_metric = _config.comparison_metric,
    };

    for (int i = 0; i < _config.num_samples; i++) {
        auto renderer = render::Renderer::Create(width, height, prng_generator.CreatePrng());

        if (!renderer.has_value()) {
            return errors::report<OptimizationResult>(renderer.error());
        }

        render_payload.renderers.push_back(*renderer);
    }

    // Now run the "sample->render->optimize" loop, keeping track of how the
    // solution is performing.
    OperationTiming pgpe_timing;
    OperationTiming render_timing;
    OperationTiming sampling_timing;

    int iterations = 0;
    for (int i = 0; i < _config.max_iterations; i++) {
        // Run the sampling step
        auto sample_job = thread_pool.SubmitWithPayload<GenerateSolutionSamples>(0, optim_payload);
        auto sample_result = sample_job.get();

        if (sample_result.error) {
            return errors::report<OptimizationResult>(sample_result.error);
        }

        sampling_timing.AddSample(sample_result.time);

        // Render images from the generated samples and compute the costs.
        std::vector<threads::Job::Future> futures;
        for (int j = 0; j < _config.num_samples; j++) {
            auto render_job = thread_pool.SubmitWithPayload<RenderAndCompare>(j, render_payload);
            futures.push_back(std::move(render_job));
        }

        threads::WaitForJobs(futures);

        for (auto &render_future : futures) {
            auto result = render_future.get();

            if (result.error) {
                return errors::report<OptimizationResult>(result.error);
            }

            render_timing.AddSample(result.time);
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

        iterations++;
    }

    // Wrap things up by rendering a final image to compute the comparison cost.
    // (Will reuse one of the renderers for this since there's no reason to make
    // a new one.)
    auto solution = optimizer->GetEstimate();
    if (!solution.has_value()) {
        return errors::report<OptimizationResult>(solution.error());
    }

    render::PackedShapeCollection image_abstraction(_config.shapes, *solution);

    auto &renderer = render_payload.renderers.front();
    renderer.SetBackground(0, 0, 0);
    renderer.Render(image_abstraction);

    auto final_cost = ComputeCost(_config.comparison_metric, reference, renderer.DrawingSurface());
    if (!final_cost.has_value()) {
        return errors::report<OptimizationResult>(final_cost.error());
    }

    auto end_to_end_time = e2e_timer.GetElapsedTime();

    OptimizationResult result{
        .solution = *solution,
        .cost = *final_cost,
        .iterations = iterations,
        .shapes = _config.shapes,
        .seed = prng_generator.BaseSeed(),
        .timing =
            TimingReport{
                .total_time = end_to_end_time,
                .initialization_time = init_timing.GetTiming().total,
                .pgpe_optimization_time = FromOperationTiming(pgpe_timing),
                .solution_sampling_time = FromOperationTiming(sampling_timing),
                .rendering_time = FromOperationTiming(render_timing),
            },
    };

    return result;
}

}  // namespace abstractions

using namespace abstractions;
using namespace fmt;

format_context::iterator formatter<ImageComparison>::format(ImageComparison metric, format_context &ctx) const
{
    string_view name = "undefined";
    switch (metric)
    {
        case ImageComparison::L1Norm:
            name = "L1";
            break;
        case ImageComparison::L2Norm:
            name = "L2";
            break;
    }
    return formatter<string_view>::format(name, ctx);
}
