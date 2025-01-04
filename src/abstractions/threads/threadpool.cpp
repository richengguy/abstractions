#include "abstractions/threads/threadpool.h"

#include <abstractions/console.h>
#include <abstractions/errors.h>

#include <chrono>

namespace abstractions::threads {

namespace {

static const std::string kConsoleName = "ThreadPool";

void WaitForQueueEmpty(Queue &queue) {
    while (queue.Size() > 1) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

}  // namespace

ThreadPool::ThreadPool(const ThreadPoolConfig &config) :
    _job_queue{config.queue_depth},
    _debug{config.debug} {
    Console console(kConsoleName);

    // Determine a "good" number of threads to use (if no value is provided).
    const int available_threads = std::thread::hardware_concurrency();
    const int default_max_threads = std::max(1, static_cast<int>(0.75 * available_threads));
    const int requested_workers = config.num_workers.value_or(default_max_threads);
    abstractions_assert(requested_workers > 0);

    if (_debug) {
        console.Print("Workers:    {}", requested_workers);
        console.Print("Queue Size: {}", _job_queue.MaxCapacity());
        console.Print("Sleep Time: {}", config.sleep_time.value_or(kDefaultWorkerSleep));
        console.Separator();
    }

    // Start all of the workerers
    for (int i = 0; i < requested_workers; i++) {
        auto &worker = _workers.emplace_back(i, _debug);

        if (config.sleep_time) {
            worker.SetSleepTime(*config.sleep_time);
        }

        worker.Start(_job_queue);

        if (_debug) {
            console.Print("Started worker {}", i);
        }
    }

    if (_debug) {
        console.Separator();
    }
}

ThreadPool::~ThreadPool() {
    Console console(kConsoleName);

    if (_debug) {
        console.Print("Waiting for queue to be empty.");
    }
    WaitForQueueEmpty(_job_queue);

    for (auto &worker : _workers) {
        worker.Stop();
        if (_debug) {
            console.Print("Stopping worker {}", worker.Id());
        }
    }
}

Job::Future ThreadPool::Submit(Job &job) {
    Console console(kConsoleName);

    if (_debug) {
        console.Print("Submitting Job#{}", job.Index());
    }

    Job::Promise status_promise;
    Job::Future status_future = status_promise.get_future();
    job.SetPromise(status_promise);
    _job_queue.Enqueue(job);
    return status_future;
}

void ThreadPool::StopAll() {
    _job_queue.Clear();
}

int ThreadPool::Workers() const {
    return _workers.size();
}

const Worker &ThreadPool::GetWorker(int i) const {
    abstractions_assert(i >= 0 && i < _workers.size());
    return _workers.at(i);
}

}  // namespace abstractions::threads
