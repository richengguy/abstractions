#include <abstractions/types.h>
#include <doctest/doctest.h>

namespace {

enum class Test { First, Second, Third };

ABSTRACTIONS_OPTIONS_ENUM(Test)

}  // namespace

TEST_SUITE_BEGIN("types");

TEST_CASE("Able to use scoped enums as bitmasks.") {
    using abstractions::Options;

    SUBCASE("Empty options should be 'false'.") {
        Options<Test> test_options;
        CHECK_FALSE(test_options);
        CHECK(!test_options);
    }

    SUBCASE("Check that the second option is set.") {
        Options<Test> test_options;
        auto set_second = test_options | Test::Second;
        CHECK(set_second);

        bool is_second_set = set_second & Test::Second;
        bool is_first_set = set_second & Test::First;
        CHECK(is_second_set == true);
        CHECK(is_first_set == false);
    }

    SUBCASE("Check that options can be chained.") {
        Options<Test> test_options = Test::First | Test::Third;
        CHECK(test_options);

        bool is_first_set = test_options & Test::First;
        bool is_second_set = test_options & Test::Second;
        bool is_third_set = test_options & Test::Third;
        CHECK(is_first_set == true);
        CHECK(is_second_set == false);
        CHECK(is_third_set == true);
    }

    SUBCASE("Can set/clear bits.") {
        Options<Test> test_options;
        CHECK_FALSE(test_options);

        // set the "second" value
        test_options.Set(Test::Second);
        bool is_first_set = test_options & Test::First;
        bool is_second_set = test_options & Test::Second;
        bool is_third_set = test_options & Test::Third;
        CHECK(is_first_set == false);
        CHECK(is_second_set == true);
        CHECK(is_third_set == false);

        // set the "first" value
        test_options.Set(Test::First);
        is_first_set = test_options & Test::First;
        is_second_set = test_options & Test::Second;
        is_third_set = test_options & Test::Third;
        CHECK(is_first_set == true);
        CHECK(is_second_set == true);
        CHECK(is_third_set == false);

        // clear the "second" value
        test_options.Clear(Test::Second);
        is_first_set = test_options & Test::First;
        is_second_set = test_options & Test::Second;
        is_third_set = test_options & Test::Third;
        CHECK(is_first_set == true);
        CHECK(is_second_set == false);
        CHECK(is_third_set == false);
    }
}

TEST_SUITE_END();
