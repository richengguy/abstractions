#include "support.h"

#include <abstractions/threads/threadpool.h>

using namespace abstractions;
using namespace abstractions::threads;

std::mutex mutex;

struct SimpleJob : public IJobFunction
{
    Error operator()(int job_id, std::vector<Job> &dependencies) const override
    {
        std::lock_guard lock{mutex};
        fmt::print("-- Running job#{}", job_id);
        std::this_thread::sleep_for(std::chrono::milliseconds(25));

        return abstractions::errors::no_error;
    }
};

ABSTRACTIONS_FEATURE_TEST()
{
    console.Print("Creating thread pool.");
    ThreadPool thread_pool({
        .debug = true,
    });

    auto job1 = Job::New<SimpleJob>(0);
    auto job2 = Job::New<SimpleJob>(1);

    thread_pool.Submit(job1);
    thread_pool.Submit(job2);

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

ABSTRACTIONS_FEATURE_TEST_MAIN("threads", "Run the ThreadPool through a simple work scenario.");
