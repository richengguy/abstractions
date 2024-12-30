#include <abstractions/threads/threadpool.h>

#include "support.h"

using namespace abstractions;
using namespace abstractions::threads;

std::mutex mutex;

struct SimpleJob : public IJobFunction {
    Error operator()(JobContext &ctx) const override {
        std::unique_lock lock{mutex};
        fmt::print("-- Running job#{} ({})\n", ctx.Id(), ctx.Worker());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return abstractions::errors::no_error;
    }
};

ABSTRACTIONS_FEATURE_TEST() {
    console.Print("Creating thread pool.");
    ThreadPool thread_pool({
        .num_workers = 4,
        .queue_depth = 2,
        // .sleep_time = std::chrono::milliseconds(250),
        .debug = true,
    });

    auto job1 = Job::New<SimpleJob>(1);
    auto job2 = Job::New<SimpleJob>(2);
    auto job3 = Job::New<SimpleJob>(3);
    auto job4 = Job::New<SimpleJob>(4);

    auto future1 = thread_pool.Submit(job1);
    auto future2 = thread_pool.Submit(job2);
    auto future3 = thread_pool.Submit(job3);
    auto future4 = thread_pool.Submit(job4);
    auto future5 = thread_pool.Submit<SimpleJob>(5);
    auto future6 = thread_pool.Submit<SimpleJob>(6);

    future1.wait();
    future2.wait();
    future3.wait();
    future4.wait();
    future5.wait();
    future6.wait();
}

ABSTRACTIONS_FEATURE_TEST_MAIN("threads", "Run the ThreadPool through a simple work scenario.");
