#include "abstractions/threads/threadpool.h"

#include <abstractions/errors.h>

namespace abstractions::threads
{

Worker::Worker()
    : _running{false}
    {}

Worker::~Worker()
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

void Worker::Start(Queue &queue)
{
    abstractions_assert(_running == false);
    _running = true;
}

void Worker::Stop()
{

}

bool Worker::IsRunning() const
{
    return _running;
}

// ThreadPool::ThreadPool(std::optional<int> num_threads)
// {
//     const int available_threads = std::thread::hardware_concurrency();
//     const int default_max_threads = std::max(1, static_cast<int>(0.75 * available_threads));
//     const int requested_threads = num_threads.value_or(default_max_threads);
//     abstractions_assert(requested_threads > 1);
// }

}
