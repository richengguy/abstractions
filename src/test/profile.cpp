#include <doctest/doctest.h>

#include <chrono>
#include <thread>

#include <abstractions/profile.h>

using namespace std::chrono_literals;

TEST_SUITE_BEGIN("profile");

TEST_CASE("Check that the timer is working correctly.")
{
    abstractions::Timer timer;

    std::this_thread::sleep_for(250ms);
    auto first_call = timer.GetElapsedTime();

    std::this_thread::sleep_for(250ms);
    auto second_call = timer.GetElapsedTime();

    // First call should be around 250ms
    CHECK(first_call > 240ms);
    CHECK(first_call < 260ms);

    // Second call should be around 500ms
    CHECK(second_call > 490ms);
    CHECK(second_call < 510ms);
}

TEST_CASE("Check that timing stats can be captured correctly.")
{
    abstractions::OperationTiming timing;

    timing.AddSample(250ms);
    timing.AddSample(250ms);

    auto stats = timing.GetTiming();
    REQUIRE(stats.count == 2);
    CHECK(stats.total == 500ms);
    CHECK(stats.mean == 250ms);
}

TEST_CASE("Check that profile scoping works as intended.")
{
    SUBCASE("Single scope works as intended.")
    {
        abstractions::OperationTiming simple_op;
        {
            abstractions::Profile profile(simple_op);
            std::this_thread::sleep_for(100ms);
        }

        auto stats = simple_op.GetTiming();
        REQUIRE(stats.count == 1);
        REQUIRE(stats.total == stats.mean);
        CHECK(stats.mean > 90ms);
        CHECK(stats.mean < 110ms);
    }

    SUBCASE("Looping works as intended.")
    {
        abstractions::OperationTiming loop_op;
        for (int i = 0; i < 5; i++)
        {
            abstractions::Profile profile(loop_op);
            std::this_thread::sleep_for(50ms);
        }

        auto stats = loop_op.GetTiming();
        REQUIRE(stats.count == 5);

        // NOTE: Only warning because this type of waiting can be flaky and it
        // shouldn't necessarily cause the test to fail.

        // Total time
        WARN(stats.total > 240ms);
        WARN(stats.total < 260ms);

        // Average time
        WARN(stats.mean > 40ms);
        WARN(stats.mean < 60ms);
    }
}

TEST_SUITE_END();
