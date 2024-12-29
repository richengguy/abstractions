#pragma once

#include <abstractions/threads/queue.h>
#include <abstractions/threads/worker.h>
#include <abstractions/types.h>

#include <optional>
#include <vector>

namespace abstractions::threads
{

/// @brief A ThreadPool configuration.
struct ThreadPoolConfig
{
    /// @brief Number of workers to create.  The default is to base the number
    ///     of workers on the number of availble CPU cores.
    std::optional<int> num_workers = {};

    /// @brief Optional job queue depth.  If provided, then the thread poll will
    ///     not accept new jobs (i.e., backpressure) when the job queue pool is
    ///     full.
    std::optional<int> queue_depth = {};

    /// @brief Enables debugging output.
    bool debug = false;
};

/// @brief A thread pool for distributing work across multiple worker threads.
///
/// The thread pool can neither be copied or moved due to an internal mutex used
/// for managing the job queue.
class ThreadPool
{
public:
    /// @brief Create a new thread pool with a default configuration.
    ThreadPool(const ThreadPoolConfig &config = ThreadPoolConfig());

    /// @brief Stop all workers and release any thread pool resources.
    ~ThreadPool();

    /// @brief Submit a job to the thread pool.  The call will block if the
    ///     internal job queue is full.
    /// @param job job for the thread pool
    void Submit(const Job &job);

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    void operator=(const ThreadPool &) = delete;
    void operator=(ThreadPool &&) = delete;

private:
    Queue _job_queue;
    std::vector<Worker> _workers;
    bool _debug;
};

}
