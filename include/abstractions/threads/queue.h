#pragma once

#include <abstractions/threads/job.h>
#include <abstractions/errors.h>

#include <deque>
#include <mutex>
#include <optional>

namespace abstractions::threads
{

class Queue
{
public:
    /// @brief Create a queue with an unlimited size.
    Queue();

    /// @brief Create a queue with a maximum size.
    /// @param max_size maximum number of entries
    Queue(int max_size);

    /// @brief Push a job onto the end queue.
    /// @param job job instance
    /// @return an error if the push wasn't successful due to the queue being full
    Error Push(Job &job);

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
    bool IsFull() const;

    /// @brief The queue's current capacity.
    /// @return the capacity if the queue has a maximum size, otherwise it will
    ///     be empty
    std::optional<int> Capacity() const;

private:
    std::mutex _guard;
    std::deque<Job> _queue;
    std::optional<int> _max_size;
};

}
