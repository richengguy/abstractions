#include <abstractions/pgpe.h>

#include <cmath>
#include <optional>
#include <string>

namespace abstractions {

std::optional<std::string> PgpeOptimizerSettings::Validate() const {
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

}  // namespace abstractions
