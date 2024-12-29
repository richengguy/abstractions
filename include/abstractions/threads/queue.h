#pragma once

#include <abstractions/threads/job.h>
#include <abstractions/errors.h>

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

    /// @brief See if there an available job in the queue.
    /// @return the job, if available, otherwise it will be empty if the queue
    ///     is empty
    std::optional<Job> Peek();

    /// @brief Removes a job from the front of the queue.
    ///
    /// Use Peek() to get the job from the queue.  This will be a no-op if the
    /// queue is currently empty.
    void Pop();

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
