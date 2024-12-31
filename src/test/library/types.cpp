#include <abstractions/types.h>

#include <doctest/doctest.h>

namespace
{

enum class Test
{
    First,
    Second,
    Third
};

ABSTRACTIONS_OPTIONS_ENUM(Test)

}

TEST_SUITE_BEGIN("types");

TEST_CASE("Able to use scoped enums as bitmasks.")
{
    using abstractions::Options;

    SUBCASE("Empty options should be 'false'.")
    {
        Options<Test> test_options;
        CHECK_FALSE(test_options);
        CHECK(!test_options);
    }

    SUBCASE("Check that the second option is set.")
    {
        Options<Test> test_options;
        auto set_second = test_options | Test::Second;
        CHECK(set_second);

        bool is_second_set = set_second & Test::Second;
        bool is_first_set = set_second & Test::First;
        CHECK(is_second_set);
        CHECK_FALSE(is_first_set);
    }

    SUBCASE("Check that options can be chained.")
    {
        Options<Test> test_options = Test::First | Test::Third;
        CHECK(test_options);

        bool is_first_set = test_options & Test::First;
        bool is_second_set = test_options & Test::Second;
        bool is_third_set = test_options & Test::Third;
        CHECK(is_first_set);
        CHECK_FALSE(is_second_set);
        CHECK(is_third_set);
    }
}

TEST_SUITE_END();
