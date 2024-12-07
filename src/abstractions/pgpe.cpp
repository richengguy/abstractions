#include <abstractions/pgpe.h>

#include <cmath>
#include <expected>
#include <optional>
#include <string>

#include <abstractions/utilities.h>

#include <fmt/format.h>

namespace abstractions {

error_t PgpeOptimizerSettings::Validate() const {
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

expected_t<PgpeOptimizer> PgpeOptimizer::Create(const PgpeOptimizerSettings &settings)
{
    auto err = settings.Validate();
    if (err)
    {
        return std::unexpected(err);
    }

    return PgpeOptimizer(settings);
}

PgpeOptimizer::PgpeOptimizer(const PgpeOptimizerSettings &settings)
    : _is_initialized{true},
      _settings{settings}
{

}

expected_t<row_vec_t> PgpeOptimizer::GetEstimate() const
{
    auto err = CheckInitialized();
    if (err)
    {
        return errors::report<row_vec_t>(err);
    }

    return _current_state;
}

expected_t<row_vec_t> PgpeOptimizer::GetSolutionStdDev() const
{
    auto err = CheckInitialized();
    if (err)
    {
        return errors::report<row_vec_t>(err);
    }

    return _current_standard_deviation;
}

expected_t<row_vec_t> PgpeOptimizer::GetSolutionVelocity() const
{
    auto err = CheckInitialized();
    if (err)
    {
        return errors::report<row_vec_t>(err);
    }

    return _current_velocity;
}

const PgpeOptimizerSettings &PgpeOptimizer::GetSettings() const
{
    return _settings;
}

void PgpeOptimizer::Initialize(const_row_vec_ref_t x_init)
{
    const int num_dim = x_init.cols();
    const double stddev_magnitdue = _settings.init_search_radius * _settings.max_speed;
    const double stddev_unit_norm = 1.0 / std::sqrt(num_dim);

    _current_state = x_init;
    _current_standard_deviation = row_vec_t::Ones(num_dim) * stddev_magnitdue * stddev_unit_norm;
    _current_velocity = row_vec_t::Zero(num_dim);
    _is_initialized = true;
}

error_t PgpeOptimizer::Sample(matrix_ref_t samples) const
{
    auto err = errors::find_any({CheckInitialized(), ValidateSamples(samples)});
    if (err)
    {
        return err;
    }

    // Fill this in
    return {};
}

error_t PgpeOptimizer::Update(const_matrix_ref_t samples, const_col_vec_ref_t costs)
{
    const int num_samples = samples.rows();
    auto err = errors::find_any({CheckInitialized(), ValidateSamples(samples), ValidateCosts(num_samples, costs)});

    // Fill this in
    return {};
}

error_t PgpeOptimizer::CheckInitialized() const
{
    if (!_is_initialized)
    {
        return "Cannot perform operation; pptimizer has not been initialized.";
    }

    return {};
}

error_t PgpeOptimizer::ValidateCosts(int num_samples, const_col_vec_ref_t costs) const
{
    if (costs.rows() != num_samples)
    {
        return fmt::format("The number of costs ({}) doesn't match the number of samples ({}).", costs.rows(), num_samples);
    }

    return {};
}

error_t PgpeOptimizer::ValidateSamples(const_matrix_ref_t samples) const
{
    const int num_samples = samples.rows();
    const int num_params = samples.cols();

    if (num_samples == 0 || (num_samples % 2) != 0)
    {
        return fmt::format("Samples matrix has {} rows; it must be greater than zero and even.", num_samples);
    }

    if (num_params != _current_state.cols())
    {
        return fmt::format("Number of columns in samples matrix ({}) does not match the size of the parameters vector ({}).", num_params, _current_state.cols());
    }

    return {};
}

}  // namespace abstractions
