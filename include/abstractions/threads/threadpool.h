#pragma once

#include <abstractions/threads/queue.h>
#include <abstractions/threads/worker.h>
#include <abstractions/types.h>

#include <future>
#include <optional>
#include <vector>

namespace abstractions::threads {

/// @brief A ThreadPool configuration.
///
/// All configuration values are optional.  Setting a value will override the
/// default.
struct ThreadPoolConfig {
    /// @brief Number of workers to create.  The default is to base the number
    ///     of workers on the number of availble CPU cores.
    std::optional<int> num_workers = {};

    /// @brief Optional job queue depth.  If provided, then the thread poll will
    ///     not accept new jobs (i.e., backpressure) when the job queue pool is
    ///     full.
    std::optional<int> queue_depth = {};

    /// @brief Specify how long workers should sleep for while waiting for new
    ///     jobs.
    std::optional<std::chrono::microseconds> sleep_time = {};

    /// @brief Enables debugging output.
    bool debug = false;
};

/// @brief A thread pool for distributing work across multiple worker threads.
///
/// The thread pool can neither be copied or moved due to an internal mutex used
/// for managing the job queue.
class ThreadPool {
public:
    /// @brief Create a new thread pool with a default configuration.
    ThreadPool(const ThreadPoolConfig &config = ThreadPoolConfig());

    /// @brief Stop all workers and release any thread pool resources, waiting
    ///     for all jobs to finish.
    ~ThreadPool();

    /// @brief Submit a job to the thread pool.  The call will block if the
    ///     internal job queue is full.
    /// @tparam T IJobFunction class type
    /// @tparam Arg IJobFunction constructor argument types
    /// @param index user-specified job ID
    /// @param args constructor arguments
    /// @return an awaitable future for the results of the job
    template <typename T, typename... Arg>
    Job::Future Submit(int index, Arg &&...args) {
        auto job = Job::New<T>(index, std::forward<Arg>(args)...);
        return Submit(job);
    }

    /// @brief Submit a job to the thread pool.  The call will block if the
    ///     internal job queue is full.
    /// @tparam T IJobFunction class type
    /// @tparam S payload type
    /// @tparam Arg IJobFunction constructor argument types
    /// @param index user-specified job ID
    /// @param payload data the job can access when it executes
    /// @param args constructor arguments
    /// @return an awaitable future for the results of the job
    template <typename T, typename S, typename... Arg>
    Job::Future SubmitWithPayload(int index, S &&payload, Arg &&...args) {
        auto job = Job::NewWithPayload<T>(index, payload, std::forward<Arg>(args)...);
        return Submit(job);
    }

    /// @brief Submit a job to the thread pool.  The call will block if the
    ///     internal job queue is full.
    /// @param job job for the thread pool
    /// @return A future with the result of the job.
    Job::Future Submit(Job &job);

    /// @brief Stop all running jobs.
    ///
    /// Any jobs that workers are *currently* executing will complete, but any
    /// jobs still in the job queue will be cancelled.  The job queue will also
    /// be cleared out.
    void StopAll();

    /// @brief Return the number of workers in the thread pool.
    int Workers() const;

    /// @brief Get a particular worker.
    /// @param i worker ID
    /// @return a constant reference to a worker
    const Worker &GetWorker(const int i) const;

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    void operator=(const ThreadPool &) = delete;
    void operator=(ThreadPool &&) = delete;

private:
    Queue _job_queue;
    std::vector<Worker> _workers;
    bool _debug;
};

}  // namespace abstractions::threads
