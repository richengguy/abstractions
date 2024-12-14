#include <abstractions/math/matrices.h>
#include <abstractions/math/random.h>
#include <abstractions/pgpe.h>
#include <abstractions/utilities.h>
#include <fmt/format.h>

#include <cmath>
#include <expected>
#include <optional>
#include <string>

namespace abstractions {

namespace {

RowVector ClipUp(ConstRowVectorRef velocity, ConstRowVectorRef &x_grad, const double v_max,
                 const double momentum) {
    const float alpha = v_max / 2.0f;
    RowVector velocity_next = momentum * velocity + alpha * (x_grad / x_grad.norm());
    float velocity_norm = velocity_next.norm();
    return velocity_norm > v_max ? v_max * velocity_next.normalized() : velocity_next;
}

RowVector ComputeStateVectorGradient(ConstColumnVectorRef costs, ConstRowVectorRef offsets,
                                     ConstRowVectorRef x_current) {
    const int random_samples = costs.rows() / 2;

    ColumnVector costs_difference =
        (costs.topRows(random_samples) - costs.bottomRows(random_samples)) / 2.0;
    return (costs_difference.asDiagonal() * offsets).colwise().sum() /
           static_cast<double>(random_samples);
}

RowVector ComputeStdDevVectorGradient(ConstColumnVectorRef costs, const double baseline,
                                      ConstRowVectorRef offsets, ConstRowVectorRef sigma_current) {
    const int random_samples = costs.rows() / 2;

    const RowVector average_cost =
        (costs.topRows(random_samples) + costs.bottomRows(random_samples)) / 2.0;
    const RowVector gradient_weights = average_cost.array() - baseline;
    const Matrix gradients =
        (offsets.array().pow(2).rowwise() - sigma_current.array().pow(2)).rowwise() /
        sigma_current.array();

    return (gradient_weights.asDiagonal() * gradients).colwise().sum() /
           static_cast<double>(random_samples);
}

std::tuple<const RowVector, const RowVector> ConstrainStateUpdate(ConstRowVectorRef state,
                                                                  ConstRowVectorRef velocity,
                                                                  ConstRowVectorRef x_grad,
                                                                  const double v_max,
                                                                  const double momentum) {
    const RowVector update_velocity = ClipUp(velocity, x_grad, v_max, momentum);
    const RowVector next_state = state + update_velocity;
    return std::make_tuple(next_state, update_velocity);
}

RowVector ConstrainStdDevUpdate(ConstRowVectorRef sigma_current, ConstRowVectorRef delta,
                                const double learning_rate, const double max_change) {
    const RowVector sigma_upper = (1 + max_change) * sigma_current;
    const RowVector sigma_lower = ((1 - max_change) * sigma_current).cwiseMax(1e-5);
    return (sigma_current + learning_rate * delta).cwiseMin(sigma_upper).cwiseMax(sigma_lower);
}

ColumnVector RankLinearizeCosts(ConstColumnVectorRef costs) {
    ColumnVector scaled_costs = ColumnVector::LinSpaced(costs.size(), 0, costs.size() - 1);

    double *start_ptr = scaled_costs.data();
    double *end_ptr = start_ptr + scaled_costs.size();

    std::sort(start_ptr, end_ptr, [&costs](const double a, const double b) -> bool {
        const int i = static_cast<int>(a);
        const int j = static_cast<int>(b);
        return costs(i) < costs(j);
    });

    scaled_costs = (scaled_costs / (costs.size() - 1)).array() - 0.5;

    return scaled_costs;
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

    return {};
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

PgpeOptimizer::PgpeOptimizer(const PgpeOptimizerSettings &settings, const uint64_t seed) :
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

    return {};
}

Error PgpeOptimizer::Update(ConstMatrixRef samples, ConstColumnVectorRef costs) {
    const int num_samples = samples.rows();
    auto err = errors::find_any(
        {CheckInitialized(), ValidateSamples(samples), ValidateCosts(num_samples, costs)});

    const ColumnVector scaled_costs = _settings.costs_ranking
                                          ? RankLinearizeCosts(costs)
                                          : static_cast<const ColumnVector>(costs);

    const Matrix offsets = samples.topRows(samples.rows() / 2).rowwise() - _current_state;
    const float baseline = scaled_costs.mean();

    const auto x_grad = ComputeStateVectorGradient(scaled_costs, offsets, _current_state);
    const auto sigma_grad =
        ComputeStdDevVectorGradient(scaled_costs, baseline, offsets, _current_standard_deviation);

    const auto [state_next, velocity_next] = ConstrainStateUpdate(
        _current_state, _current_velocity, x_grad, _settings.max_speed, _settings.momentum);

    const auto sigma_next =
        ConstrainStdDevUpdate(_current_standard_deviation, sigma_grad,
                              _settings.stddev_learning_rate, _settings.stddev_max_change);

    _current_state = state_next;
    _current_standard_deviation = sigma_next;
    _current_velocity = velocity_next;

    return {};
}

Error PgpeOptimizer::CheckInitialized() const {
    if (!_is_initialized) {
        return "Cannot perform operation; pptimizer has not been initialized.";
    }

    return {};
}

Error PgpeOptimizer::ValidateCosts(int num_samples, ConstColumnVectorRef costs) const {
    if (costs.rows() != num_samples) {
        return fmt::format("The number of costs ({}) doesn't match the number of samples ({}).",
                           costs.rows(), num_samples);
    }

    return {};
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

    return {};
}

}  // namespace abstractions
