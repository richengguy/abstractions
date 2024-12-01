#pragma once

#include <abstractions/types.h>

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
    float max_speed = std::numeric_limits<float>::signaling_NaN();

    /// @brief Smooth out gradient estimates by scaling and ranking the sample costs.
    bool costs_ranking = false;

    /// @brief The initial distribution search radius.
    float init_search_radius = 15;

    /// @brief Momentum used in gradient updates.
    ///
    /// The ClipUp algorithm uses momentum to preserve the relative direction of
    /// the state update vector (i.e., velocity).
    float momentum = 0.9;

    /// @brief Learning rate used to estimating the solution standard deviation.
    float stddev_learning_rate = 0.1;

    /// @brief The maximum allowable change between standard deviation updates.
    float stddev_max_change = 0.2;

    /// @brief Validate the optimizer settings.
    /// @return If the settings are invalid, then it will return the reason why they are invalid.
    error_t Validate() const;
};

}  // namespace abstractions
