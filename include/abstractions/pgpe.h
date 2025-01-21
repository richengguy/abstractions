#pragma once

#include <abstractions/math/random.h>
#include <abstractions/types.h>

#include <expected>
#include <limits>
#include <optional>
#include <string>

namespace abstractions {

/// @brief Runtime settings for the PgpeOptimizer
struct PgpeOptimizerSettings {
    /// @brief The largest possible magnitude of a parameter update vector.
    ///
    /// This value *must* be set as it's dependent on the problem that the
    /// optimizer is being asked to solve.  It constrains the largest possible
    /// update that the optimizer can make.
    double max_speed = std::numeric_limits<double>::signaling_NaN();

    /// @brief The initial distribution search radius.
    double init_search_radius = 15;

    /// @brief Momentum used in gradient updates.
    ///
    /// The ClipUp algorithm uses momentum to preserve the relative direction of
    /// the state update vector (i.e., velocity).
    double momentum = 0.9;

    /// @brief Learning rate used to estimating the solution standard deviation.
    double stddev_learning_rate = 0.1;

    /// @brief The maximum allowable change between standard deviation updates.
    double stddev_max_change = 0.2;

    /// @brief The seed used by the optimizer's internal RNG.  Will be generated
    ///     from a random source if not provided.
    std::optional<uint32_t> seed = {};

    /// @brief Validate the optimizer settings.
    /// @return If the settings are invalid, then it will return the reason why they are invalid.
    Error Validate() const;
};

/// @brief Optimize a function using Policy Gradients with Parameter-based
///     Exploration (PGPE).
///
/// This implementation includes the ClipUp extension to PGPE.  The full
/// algorithm details are available at the
/// [ClipUp project site](https://rupeshks.cc/projects/clipup.html).  It is a
/// black box-style optimizer that uses local sampling to estimate parameter
/// updates.  Each call to the optimizer will gradually move the solution
/// towards a local optima.
///
/// The user of this class is responsible for two things:
///
///   1. Maintaining the storage for the samples drawn by the optimizer.
///   2. Providing a way to determine the "fitness" of each drawn sample.
///
/// This will generally look something like
///
/// ```cpp
/// RowVector solution = InitialGuess();
/// Matrix samples = Allocate();
///
/// auto optimizer = PgpeOptimizer::New(settings);
/// optimizer.Initialize(solution);
///
/// while (!converged) {
///     optimizer.Sample(samples);
///     ColumnVector costs = EstimateCosts(samples);
///     optimizer.Update(samples, costs);
/// }
/// ```
class PgpeOptimizer {
public:
    /// @brief Create a new optimizer with the given settings.
    /// @param settings optimizer settings
    /// @return The configured optimizer or an Error instance if the creation
    ///     failed.
    static Expected<PgpeOptimizer> New(const PgpeOptimizerSettings &settings);

    /// @brief Create an optimizer from another one.
    /// @param other other optimizer
    PgpeOptimizer(const PgpeOptimizer &other) = default;

    /// @brief Get the current estimate of the best parameter vector from the optimizer.
    /// @return A row vector with the current estimate.
    [[nodiscard]]
    Expected<RowVector> GetEstimate() const;

    /// @brief Get the current estimate of the solutions standard deviation.
    /// @return A row vector storing the per-parameter standard deviations.
    [[nodiscard]]
    Expected<RowVector> GetSolutionStdDev() const;

    /// @brief Get the currently estimated optimizer velocity.
    /// @return A row vector with the current solution velocity.
    ///
    /// The velocity is a vector pointing along the currently estimated gradient
    /// but with a magnitude defined by the PgpeOptimizerSettings::max_speed
    /// option.
    [[nodiscard]]
    Expected<RowVector> GetSolutionVelocity() const;

    /// @brief Get the settings used for this optimizer.
    [[nodiscard]]
    const PgpeOptimizerSettings &GetSettings() const;

    /// @brief Replace the internl PRNG with a new with the provided seed.
    /// @param seed new PRNG seed
    ///
    /// This is mainly for when the optimizer is being used as part of a larger
    /// system.  This allows the internal PRNG to be configured with a new seed
    /// post-initialization.  This has the effect of also resetting the PRNG.
    void SetPrngSeed(DefaultRngType::result_type seed);

    /// @brief Initialize the optimizer to some starting state.
    /// @param x_init The initial state (parameters) vector
    ///
    /// Calling this has the effect of resetting the optimizer.  All of the
    /// internal state variables will be randomly initialized, regardless of
    /// whether or not the optimizer has ran at any point before.
    void Initialize(ConstRowVectorRef x_init);

    /// @brief Linearizes the costs so they are equally distributed on [-0.5, 0.5].
    /// @param[in,out] costs per-sample costs
    /// @note The costs are modified in-place.
    void RankLinearize(ColumnVectorRef costs) const;

    /// @brief Sample parameters from the current optimizer state.
    /// @param samples A reference to the matrix that will store the drawn
    ///     samples.
    /// @return An error if the samples could not be drawn.
    ///
    /// The optimizer stores parameters as row vectors, so the number of drawn
    /// samples will be equal to the number of rows in the provided matrix.  The
    /// number of columns must match the length of the vector that was passed
    /// into PgpeOptimizer::Initialize().
    Error Sample(MatrixRef samples) const;

    /// @brief Update the optimizer's internal state based on the reported sample costs.
    /// @param samples A set of state vector samples.  This has the same format
    ///     as the input to PgpeOptimizer::Samples().
    /// @param costs A column vector, where each element is the relative cost of
    ///     that particular solution.
    /// @return An error if the update failed.
    ///
    /// The optimizer knows nothing about the problem its being asked to solve.
    /// Rather, it has a strategy for exploring a solution space and finding the
    /// most optimal one.  The caller is responsible for calculating the
    /// correctness of each solution.
    Error Update(ConstMatrixRef samples, ConstColumnVectorRef costs);

private:
    PgpeOptimizer(const PgpeOptimizerSettings &settings, const uint32_t seed);

    Error CheckInitialized() const;
    Error ValidateCosts(int num_samples, ConstColumnVectorRef costs) const;
    Error ValidateSamples(ConstMatrixRef samples) const;

    bool _is_initialized;
    PgpeOptimizerSettings _settings;
    Prng<> _prng;

    RowVector _current_state;
    RowVector _current_standard_deviation;
    RowVector _current_velocity;
};

}  // namespace abstractions
