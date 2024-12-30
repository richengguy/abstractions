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

TEST_CASE("Can push/pop to the job queue.") {
    using abstractions::threads::Job;
    using abstractions::threads::Queue;

    SUBCASE("Push/pop without any limits.") {
        Queue queue;
        REQUIRE(queue.Size() == 0);
        REQUIRE_FALSE(queue.IsFull());

        for (int i = 0; i < 5; i++) {
            auto job = Job::New<NoOpJob>(i);
            queue.Enqueue(job);
            CHECK(queue.Size() == i + 1);
        }

        for (int i = 0; i < 5; i++) {
            auto job = queue.NextJob();
            REQUIRE(job);
            CHECK(job->Id() == i);
        }

        REQUIRE(queue.Size() == 0);
    }

    SUBCASE("Push/pop with limits (non-blocking version).") {
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

TEST_SUITE_END();
