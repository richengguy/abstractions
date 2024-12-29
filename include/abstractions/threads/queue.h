#pragma once

#include <abstractions/threads/job.h>
#include <abstractions/errors.h>

#include <future>
#include <deque>
#include <mutex>
#include <optional>

namespace abstractions::threads
{

/// @brief A job queue that supports concurrent push/pop operations.
class Queue
{
public:
    /// @brief Create a queue with an unlimited size.
    Queue();

    /// @brief Create a queue with a maximum size.
    /// @param max_size maximum number of entries
    /// @note Providing an empty optional is the same thing as creating a queue
    ///     of unlimited size.
    Queue(std::optional<int> max_size);

    template<typename T>
    Error Push(int id)
    {
        return Push(Job::New<T>(id));
    }

    /// @brief Push a job onto the end queue.
    /// @param job job instance
    /// @return an error if the push wasn't successful due to the queue being full
    Error Push(const Job &job);

    /// @brief See if there is a new job in the queue, removing it if there is one.
    /// @return The job, if one is available, or an empty value, if no job is
    ///     available.
    ///
    /// This is a combination peek+pop operation.  A lock protects access to the
    /// underlying storage so that only one thread at a time may see if a job is
    /// available.
    std::optional<Job> NextJob();

    /// @brief Determine if the queue is currently full.
    bool IsFull();

    /// @brief The queue's current size.
    int Size();

    /// @brief The queue's maximum capacity.
    /// @return the capacity if the queue has a maximum size, otherwise it will
    ///     be empty
    std::optional<int> MaxCapacity() const;

    Queue(const Queue &) = delete;
    Queue(Queue &&) = delete;
    void operator=(const Queue &) = delete;
    void operator=(Queue &&) = delete;

private:
    std::mutex _guard;
    std::deque<Job> _queue;
    std::optional<int> _max_size;
};

}
