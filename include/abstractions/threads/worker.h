#pragma once

#include <abstractions/threads/queue.h>

#include <chrono>
#include <memory>
#include <thread>

namespace abstractions::threads {

namespace detail {

/// @brief A worker's internal state.
struct WorkerState {
    int id;
    std::chrono::microseconds sleep_time;
    bool running;

    void RunJobs(Queue &queue) const;
};

}  // namespace detail

/// @brief Default time the a worker sleeps for when waiting for a job.
constexpr std::chrono::duration kDefaultWorkerSleep = std::chrono::microseconds(10);

/// @brief A worker thread that accepts a Job and executes it.
class Worker {
public:
    /// @brief Create a new worker.
    /// @param worker_id thread pool-assigned ID
    /// @param debug enables/disables debugging output
    Worker(int worker_id, bool debug = false);

    /// @brief Cleans up the worker, including shutting down the internal
    ///     work thread.
    ~Worker();

    /// @brief Start a worker.
    /// @param queue queue worker looks at for work
    void Start(Queue &queue);

    /// @brief Stops a worker.
    void Stop();

    /// @brief Check if a worker is still running.
    bool IsRunning() const;

    /// @brief The worker's unique ID.
    int Id() const;

    /// @brief Length of time the worker sleeps while waiting for a new job.
    std::chrono::microseconds SleepTime() const;

    /// @brief Set how long the worker should sleep while waiting for a new job.
    /// @param time sleep time, in microseconds
    ///
    /// This function cannot be called while the worker is running.  Attempting
    /// to do so will cause an assert.
    void SetSleepTime(const std::chrono::microseconds &time);

    Worker(Worker &&) = default;
    Worker &operator=(Worker &&) = default;

private:
    std::thread _thread;
    std::unique_ptr<detail::WorkerState> _state;
    bool _debug;
};

}  // namespace abstractions::threads
