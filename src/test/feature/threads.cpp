#include <abstractions/threads/threadpool.h>

#include <functional>
#include <fmt/ranges.h>

#include "support.h"

using namespace abstractions;
using namespace abstractions::threads;

std::mutex mutex;

struct SimpleJob : public IJobFunction {
    Error operator()(JobContext &ctx) const override {
        std::unique_lock lock{mutex};
        fmt::print("-- Running job#{} ({})\n", ctx.Id(), ctx.Worker());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return errors::no_error;
    }
};

struct ReadOnlyJob : public IJobFunction
{
    Error operator()(JobContext &ctx) const override {
        if (!ctx.HasData())
        {
            return "Missing data!";
        }
        std::string msg = ctx.Data<std::string>();

        std::unique_lock lock{mutex};
        fmt::print("-- Running job#{} ({}) with payload: {}\n", ctx.Id(), ctx.Worker(), msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        return errors::no_error;
    }
};

struct WriteOnlyJob : public IJobFunction
{
    Error operator()(JobContext &ctx) const override {
        auto array = std::any_cast<int *>(ctx.Data());
        std::unique_lock lock{mutex};
        fmt::print("-- Running job#{} ({}) - Old value: {}\n", ctx.Id(), ctx.Worker(), array[ctx.Id()]);

        array[ctx.Id()] = 123;
        fmt::print("-- New value: {}\n", array[ctx.Id()]);
        return errors::no_error;
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

    std::string msg = "This is some message.";
    auto future_w_msg = thread_pool.SubmitWithPayload<ReadOnlyJob>(10, msg);
    future_w_msg.wait();

    std::string msg2 = "This is another message";
    auto another_future_w_msg = thread_pool.SubmitWithPayload<ReadOnlyJob>(11, msg2);
    another_future_w_msg.wait();

    std::vector<int> array{0,1,2,3,4};
    auto future_w_update = thread_pool.SubmitWithPayload<WriteOnlyJob>(2, array.data());
    future_w_update.wait();
    console.Print("Array after thread: [{}]", fmt::join(array, ", "));
}

ABSTRACTIONS_FEATURE_TEST_MAIN("threads", "Run the ThreadPool through a simple work scenario.");
