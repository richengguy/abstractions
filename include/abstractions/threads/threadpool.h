#pragma once

#include <abstractions/types.h>

#include <functional>
#include <optional>
#include <deque>
#include <future>
#include <mutex>
#include <vector>
#include <thread>
#include <memory>

namespace abstractions
{

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
