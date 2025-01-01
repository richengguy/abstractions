#pragma once

#include <abstractions/math/random.h>
#include <abstractions/shapes.h>
#include <abstractions/types.h>

#include <optional>

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
    ///
    /// This is the most iterations that the optimizer will try before it
    /// "gives up".  It may stop early if the stop_on_convergence option is set
    /// to `true`.
    int max_iterations = 10000;

    /// @brief The number of samples to draw when calculating the reward costs.
    ///
    /// The PGPE optimizer generates a set of random samples around the current
    /// "best" solution.  It then expects the user to tell it how "good" the
    /// solutions are.
    int num_samples = 32;

    /// @brief Have the optimizer stop if it looks like it has found a possible
    ///     solution.
    ///
    /// The PGPE algorithm looks to maximize reward so it moves towards the
    /// solution with the highest cost.  It's possible for it to find that
    /// solution long before it hits the maximum number of iterations.  Setting
    /// this to `true` will have the optimizer check if the solution and costs
    /// have stabilized.
    bool stop_on_convergence = true;

    /// @brief The set of shapes to use for the image abstraction.
    ///
    /// The optimizer can support multiple shape types.  The default is just
    /// triangles, but different ones may be used and even combined.  Using
    /// multiple shapes will slow down the optimizer since it increases the
    /// length of the parameter vector.
    ///
    /// See the AbstractionShape enum for the list of available shapes.
    Options<AbstractionShape> shapes = AbstractionShape::Triangles;

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
    /// @note Each worker has its own PRNG.  Because this is the *base* seed,
    ///     each PRNG obtains its seed from this one.  Both seed *and*
    ///     num_workers must be the same for a result to be repeatable.
    ///
    /// A randomly generated seed will be used if one isn't specified.
    /// Otherwise the seed can be provided for some degree of repeatabilty.
    std::optional<DefaultRngType::result_type> seed = {};
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

    /// @brief The shapes used in the reconstruction.
    Options<AbstractionShape> shapes;

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
private:
};

}  // namespace abstractions
