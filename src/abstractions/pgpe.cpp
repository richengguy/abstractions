#include "abstractions/pgpe.h"

#include <abstractions/errors.h>
#include <abstractions/math/matrices.h>
#include <abstractions/math/random.h>
#include <fmt/format.h>

#include <cmath>
#include <expected>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

namespace abstractions {

namespace {

RowVector ClipUp(ConstRowVectorRef velocity, ConstRowVectorRef &x_grad, const double v_max,
                 const double momentum) {
    const double alpha = v_max / 2.0;
    RowVector velocity_next = momentum * velocity + alpha * (x_grad / x_grad.norm());
    float velocity_norm = velocity_next.norm();
    return velocity_norm > v_max ? v_max * velocity_next.normalized() : velocity_next;
}

}  // anonymous namespace

Error PgpeOptimizerSettings::Validate() const {
    if (std::isnan(max_speed)) {
        return "PGPE optimizer requires a maximum update speed parameter.";
    }
    if (init_search_radius < 0) {
        return "Initial search radius cannot be negative.";
    }
    if (max_speed < 0) {
        return "Maximum update speed cannot be negative.";
    }
    if (momentum < 0) {
        return "Momentum cannot be negative.";
    }
    if (stddev_learning_rate < 0) {
        return "Standard deviation learning rate cannot be negative.";
    }
    if (stddev_max_change < 0) {
        return "Standard deviation maximum change cannot be negative.";
    }

    return errors::no_error;
}

Expected<PgpeOptimizer> PgpeOptimizer::Create(const PgpeOptimizerSettings &settings) {
    auto err = settings.Validate();
    if (err) {
        return std::unexpected(err);
    }

    uint64_t seed = 0;
    if (settings.seed) {
        seed = *settings.seed;
    } else {
        seed = PrngGenerator<std::mt19937>::DrawRandomSeed();
    }

    return PgpeOptimizer(settings, seed);
}

PgpeOptimizer::PgpeOptimizer(const PgpeOptimizerSettings &settings, const uint32_t seed) :
    _is_initialized{true},
    _settings{settings},
    _prng{seed} {}

Expected<RowVector> PgpeOptimizer::GetEstimate() const {
    auto err = CheckInitialized();
    if (err) {
        return errors::report<RowVector>(err);
    }

    return _current_state;
}

Expected<RowVector> PgpeOptimizer::GetSolutionStdDev() const {
    auto err = CheckInitialized();
    if (err) {
        return errors::report<RowVector>(err);
    }

    return _current_standard_deviation;
}

Expected<RowVector> PgpeOptimizer::GetSolutionVelocity() const {
    auto err = CheckInitialized();
    if (err) {
        return errors::report<RowVector>(err);
    }

    return _current_velocity;
}

const PgpeOptimizerSettings &PgpeOptimizer::GetSettings() const {
    return _settings;
}

void PgpeOptimizer::Initialize(ConstRowVectorRef x_init) {
    const int num_dim = x_init.cols();
    const double stddev_magnitdue = _settings.init_search_radius * _settings.max_speed;
    const double stddev_unit_norm = 1.0 / std::sqrt(num_dim);

    _current_state = x_init;
    _current_standard_deviation = RowVector::Ones(num_dim) * stddev_magnitdue * stddev_unit_norm;
    _current_velocity = RowVector::Zero(num_dim);
    _is_initialized = true;
}

void PgpeOptimizer::RankLinearize(ColumnVectorRef costs) const {
    const int num_costs = costs.rows();
    std::vector<int> indices(num_costs);

    std::iota(std::begin(indices), std::end(indices), 0);
    std::sort(std::begin(indices), std::end(indices),
              [costs](const int &a, const int &b) -> bool { return costs(a) < costs(b); });

    for (int i = 0; i < num_costs; i++) {
        costs(indices[i]) = static_cast<double>(i) / (num_costs - 1) - 0.5;
    }
}

Error PgpeOptimizer::Sample(MatrixRef samples) const {
    auto err = errors::find_any({CheckInitialized(), ValidateSamples(samples)});
    if (err) {
        return err;
    }

    const int num_samples = samples.rows();
    const int num_params = samples.cols();

    // Fill the samples array with normally distributed values on N(0, 1).  The
    // bottom half is the negation of the top half to prepare to mirror the
    // samples.
    NormalDistribution normal_distribution(_prng, 0, 1);

    const int random_samples = num_samples / 2;
    samples.topRows(random_samples) = RandomMatrix(random_samples, num_params, normal_distribution);
    samples.bottomRows(random_samples) = -samples.topRows(random_samples);

    // Scale the parameters (columns) by the current standard deviation
    // estimate.
    samples.array().rowwise() *= _current_standard_deviation.array();

    // Now add the current state estimate to get the final set of samples.
    samples.rowwise() += _current_state;

    return errors::no_error;
}

Error PgpeOptimizer::Update(ConstMatrixRef samples, ConstColumnVectorRef costs) {
    auto err = errors::find_any(
        {CheckInitialized(), ValidateSamples(samples), ValidateCosts(samples.rows(), costs)});

    // Now we can do the parameter updates.  This follows Algorithm 1 from the
    // ClipUp paper.  Step 2 (build the population) is accomplished in the
    // Sample() method.  The gradient computation is the remaining steps.
    const int num_samples = samples.rows() / 2;

    // The "d+ - x_k" is common to both the solution and standard deviation
    // gradients so it can be factored out and computed first.  This is equal
    // to pertubation added to x_k since "d+ = x_k + sigma" and
    // "d- = x_k - sigma".  The mean fitness can also be computed.

    const Matrix perturbations = samples.topRows(num_samples).rowwise() - _current_state;
    const double baseline_cost = costs.mean();

    // Compute what's needed for getting the solution gradient
    const ColumnVector delta_cost =
        (costs.topRows(num_samples) - costs.bottomRows(num_samples)) / 2.0;

    // Compute what's needed for getting the standard deviation gradient
    const ColumnVector stddev_weights =
        ((costs.topRows(num_samples) + costs.bottomRows(num_samples)) / 2.0).array() -
        baseline_cost;
    const Matrix stddev_directions =
        (perturbations.array().pow(2).rowwise() - _current_standard_deviation.array().pow(2))
            .rowwise() /
        _current_standard_deviation.array();

    // Finally, compute the gradients
    const RowVector grad_solution =
        (delta_cost.asDiagonal() * perturbations).colwise().sum() / num_samples;
    const RowVector grad_stddev =
        (stddev_weights.asDiagonal() * stddev_directions).colwise().sum() / num_samples;

    // Use ClipUp to compute the updated velocity and state
    const RowVector updated_velocity =
        ClipUp(_current_velocity, grad_solution, _settings.max_speed, _settings.momentum);
    const RowVector updated_state = _current_state + updated_velocity;

    // Find the next standard deviation estimation, clamping the estimate so
    // that it never goes to zero or gets too large
    const RowVector stddev_upper = (1 + _settings.stddev_max_change) * _current_standard_deviation;
    const RowVector stddev_lower =
        ((1 - _settings.stddev_max_change) * _current_standard_deviation).cwiseMax(1e-5);
    const RowVector updated_stddev =
        (_current_standard_deviation + _settings.stddev_learning_rate * grad_stddev)
            .cwiseMin(stddev_upper)
            .cwiseMax(stddev_lower);

    _current_state = updated_state;
    _current_standard_deviation = updated_stddev;
    _current_velocity = updated_velocity;

    return errors::no_error;
}

Error PgpeOptimizer::CheckInitialized() const {
    if (!_is_initialized) {
        return "Cannot perform operation; pptimizer has not been initialized.";
    }

    return errors::no_error;
}

Error PgpeOptimizer::ValidateCosts(int num_samples, ConstColumnVectorRef costs) const {
    if (costs.rows() != num_samples) {
        return fmt::format("The number of costs ({}) doesn't match the number of samples ({}).",
                           costs.rows(), num_samples);
    }

    return errors::no_error;
}

Error PgpeOptimizer::ValidateSamples(ConstMatrixRef samples) const {
    const int num_samples = samples.rows();
    const int num_params = samples.cols();

    if (num_samples == 0 || (num_samples % 2) != 0) {
        return fmt::format("Samples matrix has {} rows; it must be greater than zero and even.",
                           num_samples);
    }

    if (num_params != _current_state.cols()) {
        return fmt::format(
            "Number of columns in samples matrix ({}) does not match the size of the parameters "
            "vector ({}).",
            num_params, _current_state.cols());
    }

    return errors::no_error;
}

}  // namespace abstractions
