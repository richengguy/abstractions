#include "support.h"

#include <abstractions/threads/threadpool.h>

using namespace abstractions;
using namespace abstractions::threads;

std::mutex mutex;

struct SimpleJob : public IJobFunction
{
    Error operator()(JobContext &ctx) const override
    {
        std::unique_lock lock{mutex};
        fmt::print("-- Running job#{} ({})\n", ctx.Id(), ctx.Worker());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        return abstractions::errors::no_error;
    }
};

struct EnqueueJob : public IJobFunction
{
    std::string name;

    EnqueueJob(const std::string &name)
        : name{name} { }

    Error operator()(JobContext &ctx) const override
    {
        std::unique_lock lock{mutex};
        fmt::print("-- Running enqueue job#{} ({}) [{}]\n", ctx.Id(), ctx.Worker(), name);
        ctx.EnqueueWork(Job::New<SimpleJob>(10));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return abstractions::errors::no_error;
    }
};

ABSTRACTIONS_FEATURE_TEST()
{
    console.Print("Creating thread pool.");
    ThreadPool thread_pool({
        .num_workers = 2,
        .sleep_time = std::chrono::milliseconds(250),
        .debug = true,
    });

    auto job1 = Job::New<SimpleJob>(0);
    auto job2 = Job::New<SimpleJob>(1);
    auto job3 = Job::New<SimpleJob>(2);
    auto job4 = Job::New<EnqueueJob>(3, "random job");
    auto job5 = Job::New<SimpleJob>(4);

    thread_pool.Submit(job1);
    thread_pool.Submit(job2);
    thread_pool.Submit(job3);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    thread_pool.Submit(job4);
    thread_pool.Submit(job5);

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

ABSTRACTIONS_FEATURE_TEST_MAIN("threads", "Run the ThreadPool through a simple work scenario.");
