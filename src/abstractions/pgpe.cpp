#include <abstractions/pgpe.h>
#include <abstractions/utilities.h>
#include <fmt/format.h>

#include <cmath>
#include <expected>
#include <optional>
#include <string>

namespace abstractions {

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

    return PgpeOptimizer(settings);
}

PgpeOptimizer::PgpeOptimizer(const PgpeOptimizerSettings &settings) :
    _is_initialized{true},
    _settings{settings} {}

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

    // Fill this in
    return {};
}

Error PgpeOptimizer::Update(ConstMatrixRef samples, ConstColumnVectorRef costs) {
    const int num_samples = samples.rows();
    auto err = errors::find_any(
        {CheckInitialized(), ValidateSamples(samples), ValidateCosts(num_samples, costs)});

    // Fill this in
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
