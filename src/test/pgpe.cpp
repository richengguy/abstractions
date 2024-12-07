#include <abstractions/pgpe.h>
#include <doctest/doctest.h>

TEST_SUITE_BEGIN("pgpe");

TEST_CASE("Can validate PGPE optimizer settings.") {
    const std::string kMaxSpeedNotSet = "PGPE optimizer requires a maximum update speed parameter.";

    SUBCASE("Empty option returned when settings are correct.") {
        abstractions::PgpeOptimizerSettings settings;
        settings.max_speed = 1.0;

        auto err = settings.Validate();
        REQUIRE(err.has_value() == false);
    }

    SUBCASE("Error when max_speed isn't set.") {
        abstractions::PgpeOptimizerSettings settings;

        auto err = settings.Validate();
        REQUIRE(err.has_value() == true);
        REQUIRE(*err == kMaxSpeedNotSet);
    }

    SUBCASE("Error when max_speed is negative.") {
        abstractions::PgpeOptimizerSettings settings{.max_speed = -1};

        auto err = settings.Validate();
        REQUIRE(err.value() != kMaxSpeedNotSet);
    }

    SUBCASE("Error when another value is negative.") {
        abstractions::PgpeOptimizerSettings settings{
            .max_speed = 1,
            .init_search_radius = -1,
        };

        auto err = settings.Validate();
        REQUIRE(err.value() != kMaxSpeedNotSet);
    }
}

TEST_CASE("Can create an optimizer using PgpeOptimizer::Create()") {
    SUBCASE("Error when settings are invalid.") {
        abstractions::PgpeOptimizerSettings settings{};
        auto optim = abstractions::PgpeOptimizer::Create(settings);
        REQUIRE_FALSE(optim.has_value());
    }

    SUBCASE("No error when the settings are valid.") {
        abstractions::PgpeOptimizerSettings settings{
            .max_speed = 1,
            .momentum = 123,
        };
        auto optim = abstractions::PgpeOptimizer::Create(settings);
        REQUIRE(optim.has_value());
        CHECK(optim->GetSettings().max_speed == 1);
        CHECK(optim->GetSettings().momentum == 123);
    }
}

TEST_SUITE_END();
