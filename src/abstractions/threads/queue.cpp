#include "abstractions/threads/queue.h"

#include <fmt/format.h>

namespace abstractions::threads {

Queue::Queue() {
    // don't do anything
}

Queue::Queue(std::optional<int> max_size) :
    _max_size{max_size} {
    if (_max_size) {
        abstractions_assert(*_max_size > 0);
    }
}

void Queue::Enqueue(Job &job) {
    std::unique_lock lock{_guard};

    // Wait for someone to get a job from the queue if it's full to make some
    // space for the new job.
    if (QueueFull()) {
        _space_available.wait(lock);
    }

    _queue.push_back(std::move(job));
}

Error Queue::TryEnqueue(Job &job) {
    std::unique_lock lock{_guard};

    if (_max_size && _queue.size() >= *_max_size) {
        return fmt::format("Pushing job would exceed queue capacity of {}.", *_max_size);
    }

    _queue.push_back(std::move(job));
    return errors::no_error;
}

std::optional<Job> Queue::NextJob() {
    std::unique_lock lock{_guard};
    if (_queue.empty()) {
        return {};
    }

    std::optional<Job> job(std::move(_queue.front()));
    _queue.pop_front();
    lock.unlock();  // <-- using since using an inner scope makes things a little messy

    _space_available.notify_one();
    return job;
}

void Queue::Clear() {
    std::unique_lock lock{_guard};
    _queue.clear();
}

bool Queue::IsFull() {
    std::unique_lock lock{_guard};
    return QueueFull();
}

int Queue::Size() {
    std::unique_lock lock{_guard};
    return _queue.size();
}

std::optional<int> Queue::MaxCapacity() const {
    return _max_size;
}

}  // namespace abstractions::threads
