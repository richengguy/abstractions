#include "abstractions/threads/queue.h"

#include <fmt/format.h>

namespace abstractions::threads
{

Queue::Queue()
{
    // don't do anything
}

Queue::Queue(std::optional<int> max_size)
    : _max_size{max_size}
{
    if (_max_size)
    {
        abstractions_assert(*_max_size > 0);
    }
}

Error Queue::Push(const Job &job)
{
    std::unique_lock lock{_guard};

    if (_max_size && _queue.size() >= *_max_size)
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

    return _queue.front();
}

void Queue::Pop()
{
    std::unique_lock lock{_guard};
    if (_queue.empty())
    {
        return;
    }

    _queue.pop_front();
}

bool Queue::IsFull()
{
    return _max_size && Size() >= *_max_size;
}

int Queue::Size()
{
    std::unique_lock lock{_guard};
    return _queue.size();
}

std::optional<int> Queue::MaxCapacity() const
{
    return _max_size;
}

}
