#include "abstractions/threads/worker.h"

#include <abstractions/console.h>

#include <chrono>

using namespace std::chrono_literals;

namespace abstractions::threads
{

static const std::string kConsoleName = "Worker";

namespace detail
{

void WorkerState::RunJobs(Queue &queue) const
{
    while (true)
    {
        // Not running, so need to break out of the loop.
        if (!running)
        {
            break;
        }

        // Check if there's something to draw from the queue.  If not, wait
        // for a short time before trying again.  This is to avoid the
        // thread from doing any unnecessary work.
        auto job = queue.NextJob();
        if (!job)
        {
            std::this_thread::yield();
            std::this_thread::sleep_for(sleep_time);
            continue;
        }

        // There is a job, so run it.  Errors shouldn't happen, so if the
        // worker encounters one it should cause the program to halt.
        auto results = job->Run(id);
        abstractions_check(results.error);
    }
}

}

Worker::Worker(int worker_id, bool debug)
    : _state{std::make_unique<detail::WorkerState>()},
      _debug{debug}
{
    _state->id = worker_id;
    _state->sleep_time = kDefaultWorkerSleep;
    _state->running = false;
}

Worker::~Worker()
{
    Stop();
}

void Worker::Start(Queue &queue)
{
    abstractions_assert(static_cast<bool>(_state) == true);
    abstractions_assert(_state->running == false);
    _state->running = true;

    if (_debug)
    {
        Console console(kConsoleName);
        console.Print("Starting worker {}.", _state->id);
    }

    _thread = std::thread(&detail::WorkerState::RunJobs, _state.get(), std::ref(queue));
}

void Worker::Stop()
{
    // Nothing to do if the state is empty.  This can happen when a Worker
    // instance has been moved, such as with std::vector::emplace_back() or
    // std::move().
    if (!_state)
    {
        return;
    }

    if (_thread.joinable())
    {
        if (_debug)
        {
            Console console(kConsoleName);
            console.Print("Waiting to join worker {}.", _state->id);
        }
        _state->running = false;
        _thread.join();
    }
}

bool Worker::IsRunning() const
{
    abstractions_assert(static_cast<bool>(_state) == true);
    return _state->running;
}

int Worker::Id() const
{
    abstractions_assert(static_cast<bool>(_state) == true);
    return _state->id;
}

std::chrono::microseconds Worker::SleepTime() const
{
    abstractions_assert(static_cast<bool>(_state) == true);
    return _state->sleep_time;
}

void Worker::SetSleepTime(const std::chrono::microseconds &time)
{
    abstractions_assert(static_cast<bool>(_state) == true);
    abstractions_assert(!_state->running);
    _state->sleep_time = time;
}

}
