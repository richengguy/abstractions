#include <abstractions/math/matrices.h>
#include <abstractions/math/random.h>
#include <doctest/doctest.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <vector>

TEST_SUITE_BEGIN("math");

TEST_CASE("PRNG with same seed produces the same sequence.") {
    abstractions::Prng first_prng(1);
    abstractions::Prng second_prng(1);
    abstractions::Prng third_prng(2);

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

TEST_CASE("A configured PRNG can be used with a Distribution<> object.") {
    std::normal_distribution normal_dist(1.0);
    abstractions::Prng prng(1);
    abstractions::Distribution abs_dist(prng, normal_dist);

    // Distribution mean is 1.0, so sampling 100x should be ~1.0
    double sum = 0.0;
    for (int i = 0; i < 100; i++) {
        sum += abs_dist.Sample();
    }
    sum /= 100;

    CHECK(sum == doctest::Approx(1.0).epsilon(0.01));
}

TEST_CASE("Distribution object can return the Prng<> seed.") {
    std::normal_distribution normal_dist(1.0);
    abstractions::Prng prng(123);
    abstractions::Distribution abs_dist(prng, normal_dist);

    REQUIRE(abs_dist.Seed() == 123);
}

TEST_CASE("Can create a matrix of normally distributed random values.") {
    abstractions::Prng prng(1);
    abstractions::NormalDistribution normal_dist(prng, 2.5, 1.0);
    abstractions::Matrix matrix = abstractions::RandomMatrix(25, 30, normal_dist);

    double mean = matrix.mean();
    double variance = (matrix.array() - mean).pow(2.0).mean();

    CHECK(mean == doctest::Approx(2.5).epsilon(0.05));
    CHECK(variance == doctest::Approx(1.0).epsilon(0.05));
}

TEST_CASE("Can create a matrix of uniformally distributed random values.") {
    abstractions::Prng prng(1);
    abstractions::UniformDistribution uniform_dist(prng);
    abstractions::Matrix matrix = abstractions::RandomMatrix(30, 20, uniform_dist);

    CHECK(matrix.mean() == doctest::Approx(0.5).epsilon(0.05));
}

TEST_SUITE_END();
