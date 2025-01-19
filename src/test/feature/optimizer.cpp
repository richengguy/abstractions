#include <abstractions/pgpe.h>
#include <abstractions/math/random.h>
#include <abstractions/math/matrices.h>
#include <abstractions/errors.h>

#include "support.h"

ABSTRACTIONS_FEATURE_TEST()
{
    // The Rastrigin function has many local minima but a global minima at the
    // origin.  See https://en.wikipedia.org/wiki/Rastrigin_function for
    // details.

    // This specific test is a C++ port of
    // https://github.com/nnaisense/pgpelib/blob/release/examples/01-rastrigin.ipynb

    constexpr int kNumDim = 100;
    constexpr int kNumSamples = 200;
    constexpr int kNumIter = 5000;

    auto rastrigin_fn = [](ConstRowVectorRef x) -> double
    {
        Eigen::ArrayXd x_sq = x.array().pow(2);
        Eigen::ArrayXd cos_2pi_x = Eigen::cos(2.0 * std::numbers::pi * x.array());

        return 10 * kNumDim + (x_sq - 10 * cos_2pi_x).sum();
    };

    // Create the input vector to be somewhere in a hypercube where each
    // dimension is on [-5.12, 5.12].
    UniformDistribution uniform_distribution(prng);
    RowVector initial_solution = 10.24 * RandomMatrix(1, kNumDim, uniform_distribution).array() - 5.12;

    // Create the optimizer and initialize it.
    auto optimizer = PgpeOptimizer::New({.max_speed = 0.1, .seed = prng.seed() + 1});
    abstractions_check(optimizer);
    optimizer->Initialize(initial_solution);

    console.Print("Initial Guess:");
    console.Print("sln: {}", initial_solution);
    console.Print("vel: {}", *optimizer->GetSolutionVelocity());
    console.Print("std: {}", *optimizer->GetSolutionStdDev());
    console.Separator();

    // Now run the optimization loop.
    console.Print("Running optimization...");
    Matrix samples = Matrix::Zero(kNumSamples, kNumDim);
    ColumnVector costs = ColumnVector::Zero(kNumSamples);
    for (int i = 0; i < kNumIter; i++)
    {
        abstractions_check(optimizer->Sample(samples));

        for (int j = 0; j < kNumSamples; j++)
        {
            costs(j) = -rastrigin_fn(samples.row(j));
        }

        optimizer->RankLinearize(costs);

        abstractions_check(optimizer->Update(samples, costs));

        if (i % 50 == 0)
        {
            auto solution = optimizer->GetEstimate();
            abstractions_check(solution);
            console.Print("{} -> {}", i, rastrigin_fn(*solution));
            console.Print("sln:  {}", *solution);
            console.Print("vel:  {}", *optimizer->GetSolutionVelocity());
            console.Print("std:  {}", *optimizer->GetSolutionStdDev());
        }
    }

    // Finally, check if the optimizer is close to the expected result.
    auto solution = optimizer->GetEstimate();
    abstractions_check(solution);

    auto final_cost = rastrigin_fn(*solution);
    auto distance = (*solution).norm();
    console.Separator();
    console.Print("Cost:     {}", final_cost);
    console.Print("Distance: {}", distance);

    if (kNumDim <= 5)
    {
        console.Print("Solution: [{}]", *solution);
    }
}

ABSTRACTIONS_FEATURE_TEST_MAIN("optimizer", "Uses the PGPE optimizer to find the Rastrigin function minimum.")
