#include "abstractions/threads/queue.h"

#include <fmt/format.h>

namespace abstractions::threads
{

Queue::Queue()
{
    // don't do anything
}

Queue::Queue(int max_size)
    : _max_size{max_size}
{
}

Error Queue::Push(Job &job)
{
    std::unique_lock lock{_guard};

    if (_max_size && *_max_size >= _queue.size())
    {
        return fmt::format("Pushing job would exceed queue capacity of {}.", *_max_size);
    }

    _queue.push_back(job);
    return errors::no_error;
}

std::optional<Job> Queue::Peek()
{
    std::unique_lock lock{_guard};

    if (_queue.empty())
    {
        return {};
    }

    auto job = _queue.front();
    return _queue.front();
}

}
