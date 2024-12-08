#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>

namespace abstractions {

namespace detail {

using Clock = std::chrono::steady_clock;
using Duration = std::chrono::microseconds;
using TimePoint = std::chrono::time_point<Clock>;

}  // namespace detail

/// @brief A basic timer that reports the time since it was created.
class Timer {
public:
    /// @brief Create a new timer and record the starting time.
    Timer() :
        _start{detail::Clock::now()} {}

    /// @brief Returns the time since the timer was created.
    /// @return The elapsed time, in microseconds.
    [[nodiscard]] detail::Duration GetElapsedTime() const {
        auto now = detail::Clock::now();
        return std::chrono::duration_cast<detail::Duration>(now - _start);
    }

private:
    detail::TimePoint _start;
};

/// @brief Compute the timing statistics for some repeated operation.
///
/// Use with ProfileScope() to automatically track a complex operation
/// operation.
class OperationTiming {
public:
    /// @brief The timing statistics for an observed operation.
    struct Timing {
        /// @brief The total length of time the operation has been tracked.
        detail::Duration total;

        /// @brief The mean duration of the observed operations.
        detail::Duration mean;

        /// @brief The standard deviation of the duration for the observed operations.
        detail::Duration stddev;

        /// @brief The number of times the operation timing has been sampled.
        int64_t count;
    };

    OperationTiming() :
        _total{0},
        _count{0},
        _mean{0},
        _welford_m2{0} {}

    /// @brief Add a timing sample to update the timing statistics.
    /// @param sample The time a particular operation took.
    void AddSample(const detail::Duration &sample) {
        using namespace std::chrono_literals;

        _total += sample;
        _count += 1;

        if (_count == 1) {
            _mean = sample;
            _welford_m2 = 0us;
        } else {
            detail::Duration d1, d2;
            d1 = sample - _mean;
            _mean = _mean + d1 / _count;
            d2 = sample - _mean;
            _welford_m2 = _welford_m2 + d1.count() * d2;
        }
    }

    /// @brief Get the currently estimated timing statistics.  This is updated with calls to
    /// AddSample().
    /// @return Current timing statistics.
    [[nodiscard]] Timing GetTiming() const {
        using namespace std::chrono_literals;
        auto var = _count > 0 ? _welford_m2 / _count : 0us;
        auto std = std::sqrt(var.count());

        return Timing{
            .total = _total,
            .mean = _mean,
            .stddev = detail::Duration{static_cast<detail::Duration::rep>(std)},
            .count = _count,
        };
    }

private:
    detail::Duration _total;

    // Used for Welford's algorithm.  See
    // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
    int64_t _count;
    detail::Duration _mean;
    detail::Duration _welford_m2;
};

/// @brief Tracks the time an operation took within a single scope.
///
/// This takes advantage of RAII to automatically start and sample a timer when
/// the object is created and then the program execution leaves the current
/// scope.
class Profile {
public:
    Profile(OperationTiming &profiler) :
        _profiler{profiler},
        _timer{} {}

    ~Profile() {
        _profiler.AddSample(_timer.GetElapsedTime());
    }

private:
    OperationTiming &_profiler;
    Timer _timer;
};

}  // namespace abstractions
