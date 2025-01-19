#pragma once

#include <abstractions/image.h>
#include <abstractions/math/random.h>
#include <abstractions/pgpe.h>
#include <abstractions/render/shapes.h>
#include <abstractions/types.h>
#include <fmt/base.h>

#include <functional>
#include <optional>
#include <vector>

namespace abstractions {

/// @brief The type of image comparison the abstraction engine should use.
enum class ImageComparison {
    /// @brief Compare images using the sum of absolute differences.
    L1Norm,

    /// @brief Compare images using the sum of squared differences.
    L2Norm
};

/// @brief Engine configuration options.
struct EngineConfig {
    /// @brief Total number of optimizer iterations.
    int iterations = 10000;

    /// @brief The number of samples to draw when calculating the reward costs.
    ///
    /// The PGPE optimizer generates a set of random samples around the current
    /// "best" solution.  It then expects the user to tell it how "good" the
    /// solutions are.
    int num_samples = 256;

    /// @brief The set of shapes to use for the image abstraction.
    ///
    /// The optimizer can support multiple shape types.  The default is just
    /// triangles, but different ones may be used and even combined.  Using
    /// multiple shapes will slow down the optimizer since it increases the
    /// length of the parameter vector.
    ///
    /// See the AbstractionShape enum for the list of available shapes.
    Options<render::AbstractionShape> shapes = render::AbstractionShape::Triangles;

    /// @brief The number of shapes, per shape type, to draw.
    ///
    /// This, along with the shapes option, controls how "abstract" the final
    /// result will be.  This number is the number of shapes that the engine is
    /// allowed to use when reconstructing the input image.  The *total* number
    /// of shapes will always be this value times the number of shape types.
    int num_drawn_shapes = 50;

    /// @brief The image comparison metric used to calculate the matching cost.
    ///
    /// The metric affects how fine details in the image are treated.
    ImageComparison comparison_metric = ImageComparison::L2Norm;

    /// @brief The number of worker threads used during the optimization.
    ///
    /// The default is to let the internal thread pool pick the number of
    /// workers.
    std::optional<int> num_workers = {};

    /// @brief Set the base seed for the PRNGs used by the optimizer.
    /// @note Each sample renderer has its own PRNG.  Because this is the *base*
    ///     seed, each PRNG obtains its seed from this one.  Both seed *and*
    ///     num_samples must be the same for a result to be repeatable.
    ///
    /// A randomly generated seed will be used if one isn't specified.
    /// Otherwise the seed can be provided for some degree of repeatabilty.
    std::optional<DefaultRngType::result_type> seed = {};

    /// @brief Validate the Engine configuration.
    /// @return an error if the configuration was invalid
    Error Validate() const;
};

/// @brief The timing for each stage of the optimization pipeline.
struct TimingReport {
    /// @brief Alias for "microseconds".
    using Duration = std::chrono::microseconds;

    /// @brief Information about a repeated process.
    struct ProcessTime {
        /// @brief The total time spent on this one process.
        Duration total;

        /// @brief The mean time for a single iteration of the process.
        Duration mean;

        /// @brief The standard deviation for a single iteration of the process.
        Duration standard_deviation;
    };

    /// @brief The total time the abstraction generation took.
    Duration total_time;

    /// @brief The time needed to initialize the abstraction engine.
    Duration initialization_time;

    /// @brief The time spent by the PGPE optimizer.
    ProcessTime pgpe_optimization_time;

    /// @brief The time spent generating solution samples.
    ProcessTime solution_sampling_time;

    /// @brief The time spent rendering and comparing abstract images to the
    ///     source image.
    ProcessTime rendering_time;
};

/// @brief The results of an optimization from the abstractions Engine.
struct OptimizationResult {
    /// @brief The solution that best represents the input image.
    RowVector solution;

    /// @brief The final optimization cost.
    double cost;

    /// @brief The number of iterations the optimization ran for.
    int iterations;

    /// @brief The shapes used in the reconstruction.
    Options<render::AbstractionShape> shapes;

    /// @brief The PRNG seed used by the optimizer.
    DefaultRngType::result_type seed;

    /// @brief Details about how long each stage in the abstraction process took.
    TimingReport timing;
};

/// @brief Given an image, generate an abstract representation using simple
///     shapes.
///
/// The Engine uses the PgpeOptimizer to find the "optimal" combination of
/// shapes to represent an image.  The end result will always be slightly
/// different because PGPE is a stochastic optimization algorithm; it uses
/// random sample to move towards a local optima.
class Engine {
public:
    /// @brief Create a new abstractions engine.
    /// @param config engine configuration
    /// @param optim_settings optional optimizer configuration
    /// @return the initialized engine or an error if the configuraiton failed
    static Expected<Engine> Create(const EngineConfig &config,
                                   const PgpeOptimizerSettings &optim_settings = PgpeOptimizerSettings());

    /// @brief Generate an abstract representation from the provided reference image.
    /// @param reference reference image
    /// @return the results of the optimization, or an error if the optimization
    ///     failed
    [[nodiscard]]
    Expected<OptimizationResult> GenerateAbstraction(const Image &reference) const;

    /// @brief Set the callback that runs after an optimization step.
    /// @param cb a callback function that takes the current iteration, total
    ///     number of iterations, solution cost, and the current solution
    void SetCallback(const std::function<void(int, double, ConstRowVectorRef)> &cb);

private:
    Engine(const EngineConfig &config, const PgpeOptimizerSettings &settings);
    EngineConfig _config;
    PgpeOptimizerSettings _optim_settings;
    std::function<void(int, double, ConstRowVectorRef)> _callback;
};

/// @brief Render an image abstraction with the provided configuration.
/// @param width output image width
/// @param height output image height
/// @param shapes shape configuration
/// @param solution solution vector
/// @param background_colour (optional) background colour
/// @return rendered image abstraction
[[nodiscard]] Expected<Image> RenderImageAbstraction(const int width, const int height, const Options<render::AbstractionShape> shapes, ConstRowVectorRef solution, const Pixel background_colour = Pixel(0, 0, 0, 255));

}  // namespace abstractions

/// @brief Custom formatter for the ImageComparison type.
template <>
struct fmt::formatter<abstractions::ImageComparison> : fmt::formatter<string_view> {
    fmt::format_context::iterator format(abstractions::ImageComparison metric,
                                         fmt::format_context &ctx) const;
};
