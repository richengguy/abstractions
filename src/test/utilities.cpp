#include <abstractions/types.h>
#include <abstractions/utilities.h>

#include <doctest/doctest.h>

using int_expected_t = abstractions::expected_t<int>;

TEST_SUITE_BEGIN("utilities");

TEST_CASE("Errors are wrapped correctly.")
{
    using namespace abstractions;

    SUBCASE("Can report error from string.")
    {
        auto value = errors::report<int>("some error");
        CHECK(value.has_value() == false);
        CHECK(value.error() == "some error");
    }

    SUBCASE("Can wrap an error object.")
    {
        auto value = errors::report<int>(error_t{"another error"});
        CHECK(value.has_value() == false);
        CHECK(value.error() == "another error");
    }
}

TEST_CASE("Able to find the first error with errors::find_any.")
{
    using namespace abstractions;

    SUBCASE("'nullopt' when no errors.")
    {
        auto err = errors::find_any({std::nullopt, std::nullopt, std::nullopt});
        CHECK(!err);
    }

    SUBCASE("'nullopt' when empty input.")
    {
        auto err = errors::find_any({});
        CHECK(!err);
    }

    SUBCASE("Can get the first error.")
    {
        auto err = errors::find_any({error_t{"abc"}, error_t{"def"}});
        REQUIRE(err);
        CHECK(err == "abc");
    }

    SUBCASE("Can get the first error when some are passing.")
    {
        auto err = errors::find_any({std::nullopt, std::nullopt, error_t{"error"}, std::nullopt});
        REQUIRE(err);
        CHECK(err == "error");
    }
}

TEST_SUITE_END();
