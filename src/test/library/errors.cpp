// Silence the assert output for this set of tests
#define ABSTRACTIONS_ASSERTS_THROW_ONLY

#include <abstractions/errors.h>
#include <abstractions/types.h>
#include <doctest/doctest.h>

using int_expected_t = abstractions::Expected<int>;

TEST_SUITE_BEGIN("utilities");

TEST_CASE("Errors are wrapped correctly.") {
    using namespace abstractions;

    SUBCASE("Can report error from string.") {
        auto value = errors::report<int>("some error");
        CHECK(value.has_value() == false);
        CHECK(value.error() == "some error");
    }

    SUBCASE("Can wrap an error object.") {
        auto value = errors::report<int>(Error{"another error"});
        CHECK(value.has_value() == false);
        CHECK(value.error() == "another error");
    }
}

TEST_CASE("Able to find the first error with errors::find_any.") {
    using namespace abstractions;

    SUBCASE("'nullopt' when no errors.") {
        auto err = errors::find_any({std::nullopt, std::nullopt, std::nullopt});
        CHECK(!err);
    }

    SUBCASE("'nullopt' when empty input.") {
        auto err = errors::find_any({});
        CHECK(!err);
    }

    SUBCASE("Can get the first error.") {
        auto err = errors::find_any({Error{"abc"}, Error{"def"}});
        REQUIRE(err);
        CHECK(err == "abc");
    }

    SUBCASE("Can get the first error when some are passing.") {
        auto err = errors::find_any({std::nullopt, std::nullopt, Error{"error"}, std::nullopt});
        REQUIRE(err);
        CHECK(err == "error");
    }
}

TEST_CASE("Asserts detect and throw errors correctly.") {
    using abstractions::errors::AbstractionsError;

    REQUIRE_THROWS_AS(abstractions_assert(1 == 2), AbstractionsError);
    REQUIRE_NOTHROW(abstractions_assert(1 == 1));
}

TEST_CASE("Checks can determine if an expected value has a value.")
{
    using abstractions::Expected;
    using abstractions::errors::AbstractionsError;

    auto success = Expected<int>(123);
    auto failed = abstractions::errors::report<int>("Some operation failed.");

    REQUIRE_NOTHROW(abstractions_check(success));
    REQUIRE_THROWS_AS(abstractions_check(failed), AbstractionsError);
}

TEST_SUITE_END();
