#include <abstractions/errors.h>
#include <abstractions/math/matrices.h>
#include <abstractions/math/random.h>
#include <abstractions/pgpe.h>

#include <numbers>

#include "support.h"

ABSTRACTIONS_FEATURE_TEST() {
    // The Rastrigin function has many local minima but a global minima at the
    // origin.  See https://en.wikipedia.org/wiki/Rastrigin_function for
    // details.

    // This specific test is a C++ port of
    // https://github.com/nnaisense/pgpelib/blob/release/examples/01-rastrigin.ipynb

    constexpr int kNumDim = 100;
    constexpr int kNumSamples = 250;
    constexpr int kNumIter = 5000;
    constexpr double kA = 10.0;

    auto rastrigin_fn = [&](ConstRowVectorRef x) -> double {
        Eigen::ArrayXd x_sq = x.array() * x.array();
        Eigen::ArrayXd cos_2pi_x = Eigen::cos(2.0 * std::numbers::pi * x.array());

        return kA * kNumDim + (x_sq - kA * cos_2pi_x).sum();
    };

    // Create the input vector to be somewhere in a hypercube where each
    // dimension is on [-5.12, 5.12].
    UniformDistribution uniform_distribution(prng);
    RowVector initial_solution =
        10.24 * RandomMatrix(1, kNumDim, uniform_distribution).array() - 5.12;

    // Create the optimizer and initialize it.
    auto optimizer = PgpeOptimizer::New({
        .max_speed = 0.06,
        .stddev_learning_rate = 0.1,
        .stddev_max_change = 0.2,
        .seed = prng.seed() + 1,
    });
    abstractions_check(optimizer);
    optimizer->Initialize(initial_solution, 1.0);

    console.Print("Initial Guess:");
    console.Print("sln: {}", initial_solution);
    console.Print("vel: {}", *optimizer->GetSolutionVelocity());
    console.Print("std: {}", *optimizer->GetSolutionStdDev());
    console.Separator();

    // Now run the optimization loop.
    console.Print("Running optimization...");
    Matrix samples = Matrix::Zero(kNumSamples, kNumDim);
    ColumnVector costs = ColumnVector::Zero(kNumSamples);
    for (int i = 0; i < kNumIter; i++) {
        abstractions_check(optimizer->Sample(samples));

        for (int j = 0; j < kNumSamples; j++) {
            costs(j) = -rastrigin_fn(samples.row(j));
        }

        optimizer->RankLinearize(costs);

        abstractions_check(optimizer->Update(samples, costs));

        if (i % 50 == 0) {
            auto solution = optimizer->GetEstimate();
            abstractions_check(solution);
            console.Print("{} -> {}", i, rastrigin_fn(*solution));

            if (kNumDim <= 5) {
                console.Print("sln:  {}", *solution);
                console.Print("vel:  {}", *optimizer->GetSolutionVelocity());
                console.Print("std:  {}", *optimizer->GetSolutionStdDev());
            }
        }
    }

    // Finally, check if the optimizer is close to the expected result.
    auto solution = optimizer->GetEstimate();
    abstractions_check(solution);

    console.Separator();
    console.Print("Final Result:");
    console.Print("sln: {}", initial_solution);
    console.Print("vel: {}", *optimizer->GetSolutionVelocity());
    console.Print("std: {}", *optimizer->GetSolutionStdDev());

    auto final_cost = rastrigin_fn(*solution);
    auto distance = (*solution).norm();
    console.Separator();
    console.Print("Cost:     {}", final_cost);
    console.Print("Distance: {}", distance);
}

ABSTRACTIONS_FEATURE_TEST_MAIN("optimizer",
                               "Uses the PGPE optimizer to find the Rastrigin function minimum.")
