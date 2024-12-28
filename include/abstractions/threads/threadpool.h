#pragma once

#include <abstractions/threads/queue.h>
#include <abstractions/types.h>

namespace abstractions::threads
{

/// @brief A worker thread that accepts a Job and executes it.
class Worker
{
public:
    Worker();
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

// /// @brief Contains a set of jobs that will be executed on a thread pool.
// ///
// /// The queue is blocking so *new* jobs cannot be submitting while the queue is
// /// full.
// class JobQueue
// {
// public:
//     void Push(IJob &job);
//     std::optional<ConstIJobRef> Peek() const;
//     void Pop();
//     int Size() const;

// private:
//     std::deque<std::shared_ptr<IJob>> _jobs;
//     std::mutex _guard;
// };

// class WorkerThread
// {
// public:
//     WorkerThread();
//     ~WorkerThread();

//     void Start(JobQueue &queue);
//     void Stop();

// private:
//     std::atomic<bool> _stop;
//     std::thread _thread;
// };

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
