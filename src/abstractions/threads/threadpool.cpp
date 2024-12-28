#include "abstractions/threads/threadpool.h"

#include <abstractions/errors.h>

#include <chrono>

using namespace std::chrono_literals;

namespace abstractions::threads
{

// Default time the a worker sleeps for when waiting for a job.
constexpr std::chrono::duration kDefaultWait = 10us;

Worker::Worker()
    : _running{false}
    {}

Worker::~Worker()
{
    Stop();
}

void Worker::Start(Queue &queue)
{
    abstractions_assert(_running == false);
    _running = true;

    _thread = std::thread([&]()
    {
        while (true)
        {
            // Not running, so need to break out of the loop.
            if (!_running)
            {
                break;
            }

            // Check if there's something to draw from the queue.  If not, wait
            // for a short time before trying again.  This is to avoid the
            // thread from doing any unnecessary work.
            auto job = queue.Peek();
            if (!job)
            {
                std::this_thread::yield();
                std::this_thread::sleep_for(kDefaultWait);
                continue;
            }

            queue.Pop();

            // There is a job, so run it.
            std::promise<Error> job_promise;
            job->Run(std::move(job_promise));
        }
    });
}

void Worker::Stop()
{
    if (_thread.joinable())
    {
        _running = false;
        _thread.join();
    }
}

bool Worker::IsRunning() const
{
    return _running;
}


ThreadPoolConfig::ThreadPoolConfig()
{
    const int available_threads = std::thread::hardware_concurrency();
    const int default_max_threads = std::max(1, static_cast<int>(0.75 * available_threads));
    num_workers = default_max_threads;
    queue_depth = {};
}

ThreadPool::ThreadPool(const ThreadPoolConfig &config)
{
    // Do something
}

}
