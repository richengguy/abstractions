#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>

namespace abstractions {

namespace detail {

using clock_t = std::chrono::steady_clock;
using duration_t = std::chrono::microseconds;
using time_point_t = std::chrono::time_point<clock_t>;

} // namespace abstractions::detail

/// A very basic timer class.
class Timer
{
public:
    /// Create a new timer instance.
    Timer() : _start{detail::clock_t::now()} { }

    /// Get the number of microseconds since the timer first started.
    [[nodiscard]] detail::duration_t GetElapsedTime() const
    {
        auto now = detail::clock_t::now();
        return std::chrono::duration_cast<detail::duration_t>(now - _start);
    }

private:
    detail::time_point_t _start;
};

/// Computes the timing statistics on some operation.
class Profiler
{
public:
    struct Timing
    {
        detail::duration_t total;
        detail::duration_t mean;
        detail::duration_t stddev;
        int64_t count;
    };

    Profiler()
        : _total{0},
          _count{0},
          _mean{0},
          _welford_m2{0}
    { }

    void AddSample(const detail::duration_t &sample)
    {
        using namespace std::chrono_literals;

        _total += sample;
        _count += 1;

        if (_count == 1)
        {
            _mean = sample;
            _welford_m2 = 0us;
        }
        else
        {
            detail::duration_t d1, d2;
            d1 = sample - _mean;
            _mean = _mean + d1 / _count;
            d2 = sample - _mean;
            _welford_m2 = _welford_m2 + d1.count() * d2;
        }
    }

    /// Returns the per-iteration timings.
    [[nodiscard]] Timing GetTiming() const
    {
        return Timing
        {
            .total = TotalTime(),
            .mean = MeanIterationTime(),
            .stddev = IterationStandardDeviation(),
            .count = Iterations()
        };
    }

    [[nodiscard]] int64_t Iterations() const { return _count; }

    [[nodiscard]] detail::duration_t IterationStandardDeviation() const
    {
        auto stddev = std::sqrt(IterationVariance().count());
        return detail::duration_t{static_cast<detail::duration_t::rep>(stddev)};
    }

    [[nodiscard]] detail::duration_t IterationVariance() const
    {
        using namespace std::chrono_literals;
        return _count > 0 ? _welford_m2 / _count : 0us;
    }

    [[nodiscard]] detail::duration_t MeanIterationTime() const { return _mean; }

    [[nodiscard]] detail::duration_t TotalTime() const { return _total; }

private:
    detail::duration_t _total;

    // Used for Welford's algorithm.  See
    // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
    int64_t _count;
    detail::duration_t _mean;
    detail::duration_t _welford_m2;
};

/// Defines the region being profiled.
class ProfilerScope
{
public:
    ProfilerScope(Profiler &parent)
        : _is_running{true},
          _profiler{parent},
          _timer{}
    { }

    ~ProfilerScope()
    {
        LeaveScope();
    }

    void LeaveScope()
    {
        if (_is_running == false) { return; }
        _profiler.AddSample(_timer.GetElapsedTime());
        _is_running = false;
    }

private:
    bool _is_running;
    Profiler &_profiler;
    Timer _timer;
};

} // namespace abstractions
