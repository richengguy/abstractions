#pragma once

#include <abstractions/threads/queue.h>
#include <abstractions/types.h>

#include <optional>
#include <vector>

namespace abstractions::threads
{

/// @brief A worker thread that accepts a Job and executes it.
class Worker
{
public:
    /// @brief Create a new worker.
    Worker();

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

    Worker(const Worker &) = delete;
    Worker(Worker &&) = delete;
    void operator=(const Worker &) = delete;
    void operator=(Worker &&) = delete;

private:
    std::atomic<bool> _running;
    std::thread _thread;
};


/// @brief A ThreadPool configuration.
struct ThreadPoolConfig
{
    /// @brief Number of workers to create.
    int num_workers;

    /// @brief Optional job queue depth.
    std::optional<int> queue_depth;

    /// @brief Create a ThreadPool configuration with default values.
    ThreadPoolConfig();
};


/// @brief A thread pool for distributing work across multiple worker threads.
class ThreadPool
{
public:
    /// @brief Create a new thread pool with a default configuration.
    ThreadPool(const ThreadPoolConfig &config = ThreadPoolConfig());

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    void operator=(const ThreadPool &) = delete;
    void operator=(ThreadPool &&) = delete;

private:
    Queue _job_queue;
    std::vector<Worker> _workers;
};

// /// @brief A thread pool capable of performing some sort of work.
// class ThreadPool
// {
// public:

//     /// @brief Create a new thread pool.
//     /// @param num_cores number of cores to use; a default value will be used if
//     ///     not provided
//     ThreadPool(std::optional<int> num_cores = std::nullopt);

//     ThreadPool(const ThreadPool &) = delete;
//     ThreadPool(ThreadPool &&) = delete;
//     void operator=(const ThreadPool &) = delete;
//     void operator=(ThreadPool &&) = delete;

// private:
//     JobQueue _jobs;
//     std::vector<WorkerThread> _workers;
// };

}
