#include <abstractions/math/matrices.h>
#include <abstractions/math/random.h>
#include <abstractions/pgpe.h>
#include <doctest/doctest.h>

#include <Eigen/Core>
#include <numbers>

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
        auto optim = abstractions::PgpeOptimizer::New(settings);
        REQUIRE_FALSE(optim.has_value());
    }

    SUBCASE("No error when the settings are valid.") {
        abstractions::PgpeOptimizerSettings settings{
            .max_speed = 1,
            .momentum = 123,
        };
        auto optim = abstractions::PgpeOptimizer::New(settings);
        REQUIRE(optim.has_value());
        CHECK(optim->GetSettings().max_speed == 1);
        CHECK(optim->GetSettings().momentum == 123);
    }
}

TEST_CASE("Costs can be correctly rank-linearized.") {
    using abstractions::ColumnVector;
    using abstractions::Matrix;
    using abstractions::PgpeOptimizer;
    using abstractions::PgpeOptimizerSettings;

    ColumnVector costs(5);
    costs << 8, 7, 1, 9, 6;
    // Rank: 3  2  0  4  1

    ColumnVector original_costs = costs;

    ColumnVector expected(5);
    expected << 0.25, 0.0, -0.5, 0.5, -0.25;

    auto optimizer = PgpeOptimizer::New(PgpeOptimizerSettings{.max_speed = 1.0});
    optimizer->RankLinearize(costs);

    INFO("Linearized Costs: ", costs.transpose());
    INFO("Expected Costs:   ", expected.transpose());
    REQUIRE(costs == expected);
}

TEST_CASE("PgpeOptimizer can find the equation of a line from noisy data.") {
    using abstractions::ColumnVector;
    using abstractions::Matrix;
    using abstractions::NormalDistribution;
    using abstractions::PgpeOptimizer;
    using abstractions::PgpeOptimizerSettings;
    using abstractions::Prng;
    using abstractions::RandomMatrix;

    // Constants
    constexpr int kIterations = 2500;
    constexpr int kNumPoints = 100;
    constexpr int kSamples = 16;
    constexpr double kNoiseMagnitude = 0.1;
    constexpr double kInvSqrt2 = 1.0 / std::numbers::sqrt2;

    auto estimate_costs = [&](const Eigen::Matrix<double, kNumPoints, 2> &points,
                              const Eigen::Matrix<double, kSamples, 3> &solutions) -> ColumnVector {
        // The solution costs are calculated from the perpendicular point-line
        // distances.  First, the solutions are converted into Hesse-normal form
        // to make the calculation easy.  Then, the costs are found for each
        // proposed solution.  PGPE maximizes rewards so the negative cost is
        // returns, as the lower the value, the better the fit.

        const Eigen::Matrix<double, Eigen::Dynamic, 3> lines =
            solutions.array().colwise() / solutions.leftCols<2>().rowwise().norm().array();

        Eigen::Matrix<double, Eigen::Dynamic, 3> points_augmented =
            Eigen::Matrix<double, Eigen::Dynamic, 3>::Ones(points.rows(), 3);
        points_augmented.leftCols(2) = points;

        ColumnVector costs(solutions.rows());
        for (int i = 0; i < solutions.rows(); i++) {
            costs(i) = (lines.row(i) * points_augmented.transpose()).cwiseAbs().sum();
        }

        return -costs;
    };

    // Generate the ground truth and input data
    // The line's equation in implicit form is 'x/sqrt(2) + y/sqrt(2) - 5 = 0'
    const Eigen::RowVector2<double> norm(kInvSqrt2, kInvSqrt2);
    const double distance = 5;

    const Eigen::RowVector2<double> line_pt = distance * norm;
    const Eigen::RowVector2<double> direction(kInvSqrt2, -kInvSqrt2);

    Prng prng{2};
    NormalDistribution normal_dist(prng, 0, kNoiseMagnitude);
    const Eigen::Matrix<double, kNumPoints, 2> noise = RandomMatrix(kNumPoints, 2, normal_dist);

    // The expression below is just 'direction * t + line_pt + noise'.  The
    // reason it looks like this is just from the Eigen broadcasting operations.
    const Eigen::Matrix<double, kNumPoints, 1> t =
        Eigen::Matrix<double, kNumPoints, 1>::LinSpaced(-5, 5);
    const Eigen::Matrix<double, kNumPoints, 2> points =
        (direction.replicate(kNumPoints, 1).array() * t.replicate(1, 2).array() + noise.array())
            .rowwise() +
        line_pt.array();

    // Construct the optimizer and try to find a "good" solution
    auto optimizer = PgpeOptimizer::New(PgpeOptimizerSettings{.max_speed = 0.2, .seed = 3});

    INFO("Ensure optimizer is created.");
    REQUIRE(optimizer);

    // Allocate samples storage and initial solution vector
    Eigen::Matrix<double, kSamples, 3> samples = Eigen::Matrix<double, kSamples, 3>::Zero();
    samples.col(1).array() = 1;

    // Initialize the optimizer with the line 'y = 0'
    Eigen::RowVector3<double> solution(0, 1, 0);
    auto msg_solution =
        fmt::format("Initial Solution: {} {} {}", solution.x(), solution.y(), solution.z());
    INFO(msg_solution);

    optimizer->Initialize(solution);

    // Run the optimization loop
    for (int i = 0; i < kIterations; i++) {
        auto err = optimizer->Sample(samples);
        if (err) {
            INFO("Sample Error: ", err.value());
            REQUIRE(false);
        }

        ColumnVector costs = estimate_costs(points, samples);

        err = optimizer->Update(samples, costs);
        if (err) {
            INFO("Update Error: ", err.value());
            REQUIRE(false);
        }
    }

    // Get the final, predicted, solution
    auto estimate = optimizer->GetEstimate();
    REQUIRE(estimate);

    // Convert it into Hesse-normal form
    solution = *estimate;
    solution.array() /= solution.head<2>().norm();
    if (solution(2) > 0) {
        solution = -solution;
    }

    // Compute the mean absolute error and ensure it's "good enough"
    const Eigen::RowVector3<double> line(kInvSqrt2, kInvSqrt2, -distance);
    const double error = (line - solution).cwiseAbs().mean();

    INFO("Expected: ", line);
    INFO("Actual:   ", solution);
    INFO("Error:    ", error);
    REQUIRE(error < 0.25);
}

TEST_SUITE_END();
