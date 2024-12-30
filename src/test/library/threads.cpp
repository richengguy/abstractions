#include <abstractions/threads/job.h>
#include <abstractions/threads/queue.h>
#include <doctest/doctest.h>

#include <vector>

namespace {

struct NoOpJob : public abstractions::threads::IJobFunction {
    abstractions::Error operator()(abstractions::threads::JobContext &ctx) const override {
        return abstractions::errors::no_error;
    }
};

}  // namespace

TEST_SUITE_BEGIN("threads");

TEST_CASE("Can push/pop to the job queue. (Non-blocking)") {
    using abstractions::threads::Job;
    using abstractions::threads::Queue;

    SUBCASE("Push/pop without any limits.") {
        Queue queue;
        REQUIRE(queue.Size() == 0);
        REQUIRE_FALSE(queue.IsFull());

        for (int i = 0; i < 5; i++) {
            auto job = Job::New<NoOpJob>(i);
            CHECK(queue.TryEnqueue(job) == abstractions::errors::no_error);
            CHECK(queue.Size() == i + 1);
        }

        for (int i = 0; i < 5; i++) {
            auto job = queue.NextJob();
            REQUIRE(job);
            CHECK(job->Id() == i);
        }

        REQUIRE(queue.Size() == 0);
    }

    SUBCASE("Push/pop with limits.") {
        Queue queue(3);
        REQUIRE(queue.MaxCapacity());
        CHECK(queue.MaxCapacity().value() == 3);

        for (int i = 0; i < 3; i++) {
            INFO("Push job ID ", i);
            auto job = Job::New<NoOpJob>(i);
            auto err = queue.TryEnqueue(job);
            CHECK(err == abstractions::errors::no_error);
            CHECK(queue.Size() == i + 1);
        }

        CHECK(queue.IsFull());

        for (int i = 3; i < 5; i++) {
            INFO("Push extra job ", i);
            auto job = Job::New<NoOpJob>(i);
            auto err = queue.TryEnqueue(job);
            CHECK(err != abstractions::errors::no_error);
            CHECK(queue.Size() == 3);
        }

        auto job = queue.NextJob();
        REQUIRE(job);
        INFO("Popping job ", job->Id());

        CHECK_FALSE(queue.IsFull());
        INFO("Pushing new job.");
        auto new_job = Job::New<NoOpJob>(5);
        auto err = queue.TryEnqueue(new_job);
        CHECK(err == abstractions::errors::no_error);
    }
}

TEST_CASE("Can push/pop to job queue asynchronously. (Blocking)") {
    using abstractions::threads::Job;
    using abstractions::threads::Queue;

    auto parallel_work = [](Queue &queue) {
        auto job = Job::New<NoOpJob>(10);
        CHECK(queue.IsFull());
        queue.Enqueue(job);
    };

    // Fill up the queue so that any future calls will block because the queue
    // is full.
    const int kCapacity = 3;
    Queue queue(kCapacity);
    for (int i = 0; i < kCapacity; i++) {
        auto job = Job::New<NoOpJob>(i);
        REQUIRE_FALSE(queue.IsFull());
        queue.Enqueue(job);
    }

    // Now, launch a separate job that will attempt to push to the thread.  It
    // should block.
    std::thread thread(parallel_work, std::ref(queue));

    // Wait a bit to simulate doing some work.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Pull from the queue.  After waiting for a short time, the queue should
    // still be full from the worker thread.
    auto job0 = queue.NextJob();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    REQUIRE(job0);
    CHECK(job0->Id() == 0);
    CHECK(queue.IsFull());

    // Now, pull the remaining jobs. The last one should have the job created
    // from the other thread (job ID '10').
    auto job1 = queue.NextJob();
    REQUIRE(job1);
    CHECK(job1->Id() == 1);

    auto job2 = queue.NextJob();
    REQUIRE(job2);
    CHECK(job2->Id() == 2);

    auto job10 = queue.NextJob();
    REQUIRE(job10);
    CHECK(job10->Id() == 10);

    // Do the final clean-up.
    thread.join();
}

TEST_SUITE_END();
