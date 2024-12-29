#pragma once

#include <abstractions/threads/queue.h>

#include <thread>

namespace abstractions::threads
{

/// @brief A worker thread that accepts a Job and executes it.
class Worker
{
public:
    /// @brief Create a new worker.
    Worker(int worker_id);

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

    Worker(Worker &&) = default;
    Worker &operator=(Worker &&) = default;

private:
    std::thread _thread;
    bool _running;
    int _worker_id;
};

}
