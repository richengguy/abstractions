#include "abstractions/threads/threadpool.h"

#include <abstractions/errors.h>
#include <abstractions/console.h>

#include <chrono>

namespace abstractions::threads
{

namespace
{

void WaitForQueueEmpty(Queue &queue)
{
    while (queue.Size() > 1)
    {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

}

static const std::string kConsoleName = "ThreadPool";

ThreadPool::ThreadPool(const ThreadPoolConfig &config)
    : _job_queue{config.queue_depth},
      _debug{config.debug}
{
    Console console(kConsoleName);

    // Determine a "good" number of threads to use (if no value is provided).
    const int available_threads = std::thread::hardware_concurrency();
    const int default_max_threads = std::max(1, static_cast<int>(0.75 * available_threads));
    const int requested_workers = config.num_workers.value_or(default_max_threads);
    abstractions_assert(requested_workers > 0);

    if (_debug) {
        console.Print("Workers:    {}", _workers.size());
        console.Print("Queue Size: {}", _job_queue.MaxCapacity());
        console.Print("Sleep Time: {}", config.sleep_time.value_or(kDefaultWorkerSleep));
        console.Separator();
    }

    // Start all of the workerers
    for (int i = 0; i < requested_workers; i++)
    {
        auto &worker = _workers.emplace_back(i);

        if (config.sleep_time)
        {
            worker.SetSleepTime(*config.sleep_time);
        }

        worker.Start(_job_queue);

        if (_debug)
        {
            console.Print("Started worker {}", i);
        }
    }

    if (_debug)
    {
        console.Separator();
    }
}

ThreadPool::~ThreadPool()
{
    Console console(kConsoleName);

    console.Print("Waiting for queue to be empty.");
    WaitForQueueEmpty(_job_queue);
    console.Print("Queue is empty.");

    for (auto &worker : _workers)
    {
        worker.Stop();
        if (_debug)
        {
            console.Print("Stopping worker {}", worker.Id());
        }
    }
}

void ThreadPool::Submit(const Job &job)
{
    Console console(kConsoleName);

    if (_debug)
    {
        console.Print("Submitting Job#{}", job.Id());
    }

    _job_queue.Enqueue(job);
}

void ThreadPool::StopAll()
{
    _job_queue.Clear();
}

}
