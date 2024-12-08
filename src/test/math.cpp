#include <abstractions/math/random.h>
#include <doctest/doctest.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <vector>

TEST_SUITE_BEGIN("math");

TEST_CASE("PRNG with same seed produces the same sequence.") {
    abstractions::Prng<std::mt19937> first_prng(1);
    abstractions::Prng<std::mt19937> second_prng(1);
    abstractions::Prng<std::mt19937> third_prng(2);

    std::vector<uint32_t> first_sequence;
    std::vector<uint32_t> second_sequence;
    std::vector<uint32_t> third_sequence;

    for (int i = 0; i < 10; i++) {
        first_sequence.push_back(first_prng());
        second_sequence.push_back(second_prng());
        third_sequence.push_back(third_prng());
    }

    INFO(fmt::format("First Sequence:   [{}]", fmt::join(first_sequence, ", ")));
    INFO(fmt::format("Second Sequence:  [{}]", fmt::join(second_sequence, ", ")));
    INFO(fmt::format("Third Sequence:   [{}]", fmt::join(third_sequence, ", ")));

    bool first_and_second_all_same = true;
    bool first_and_third_all_same = true;
    for (int i = 0; i < 10; i++) {
        first_and_second_all_same &= first_sequence[i] == second_sequence[i];
        first_and_third_all_same &= first_sequence[i] == third_sequence[i];
    }

    REQUIRE(first_and_second_all_same);
    REQUIRE_FALSE(first_and_third_all_same);
}

TEST_CASE("PRNG generator's state can be configured correctly.") {
    abstractions::PrngGenerator generator(1);

    REQUIRE(generator.BaseSeed() == 1);
    REQUIRE(generator.SequenceNumber() == 0);

    auto prng = generator.CreatePrng();
    REQUIRE(generator.SequenceNumber() == 1);
    REQUIRE(prng.seed() == 2);
}

TEST_SUITE_END();
