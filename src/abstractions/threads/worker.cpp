#include "abstractions/threads/worker.h"

#include <abstractions/console.h>

#include <chrono>

using namespace std::chrono_literals;

namespace abstractions::threads
{

// Default time the a worker sleeps for when waiting for a job.
constexpr std::chrono::duration kDefaultWait = 10us;

Worker::Worker(int worker_id)
    : _running{false},
      _worker_id{worker_id}
    {}

Worker::~Worker()
{
    Stop();
}

void Worker::Start(Queue &queue)
{
    abstractions_assert(_running == false);
    _running = true;

    Console console("Worker");
    console.Print("Starting worker.");

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
                console.Print("{} - peeking", _worker_id);
                continue;
            }

            queue.Pop();

            // There is a job, so run it.  Errors shouldn't happen, so if the
            // worker encounters one it should cause the program to halt.
            auto results = job->Run();
            abstractions_check(results.error);

            // The job might spawn child jobs, so enqueue those if there are
            // any.
            for (auto &job : results.dependencies)
            {
                queue.Push(job);
            }
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

int Worker::Id() const
{
    return _worker_id;
}

}
