#pragma once

#include <abstractions/math/random.h>
#include <abstractions/profile.h>
#include <abstractions/console.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>

#include <CLI/CLI.hpp>
#include <filesystem>
#include <optional>

#include "test-paths.h"

/// @brief Use this to specify what the feature test actually does.
///
/// The feature test will have access to the following objects:
///
/// * `console` - Method for providing logging output.
/// * `output_folder` - An object that provides some scratch space for the test.
/// * `prng` - A PRNG instance the test can use.
#define ABSTRACTIONS_FEATURE_TEST() \
    void ::abstractions::tests::TestFixture::Func(Prng<> &prng) const

/// @brief Creates the "main()" function for a feature test in a standardized way.
/// @param name test name
/// @param desc short description shown in the commandline
#define ABSTRACTIONS_FEATURE_TEST_MAIN(name, desc)              \
    int main(int nargs, char **args) {                          \
        ::abstractions::tests::TestFixture fixture(name, desc); \
        return fixture.Run(nargs, args);                        \
    }

namespace abstractions::tests {

/// @brief Manages the output results folder for a particular feature test.
class TestOutputFolder {
public:
    TestOutputFolder(const std::string &test_name) :
        _folder{kResultsPath / "s" / test_name} {
        Console console(test_name);

        if (std::filesystem::exists(_folder)) {
            auto removed = std::filesystem::remove_all(_folder);
            console.Print("Removed {} files.", fmt::styled(removed, fmt::emphasis::bold));
        }

        auto ret = std::filesystem::create_directories(_folder);
        if (ret) {
            console.Print("Created {}", fmt::styled(_folder.c_str(), fmt::emphasis::bold));
        }
    }

    /// @brief Top-level test case folder path.
    std::filesystem::path Path() const {
        return _folder;
    }

    /// @brief Construct a file path to a file in the output folder.
    /// @param name file name
    /// @return the complete path
    std::filesystem::path FilePath(const std::string &name) const {
        return _folder / name;
    }

    TestOutputFolder(const TestOutputFolder &) = delete;
    TestOutputFolder(TestOutputFolder &&) = delete;
    void operator=(const TestOutputFolder &) = delete;
    void operator=(TestOutputFolder &&) = delete;

private:
    const std::filesystem::path _folder;
};

/// @brief Test fixture used to setup and run feature tests in a common way.
///
/// The test fixture is initialized with the ABSTRACTIONS_FEATURE_TEST_MAIN and
/// ABSTRACTIONS_FEATURE_TEST macros.  The ABSTRACTIONS_FEATURE_TEST_MAIN
/// initializes the fixture object and calls it.  The ABSTRACTIONS_FEATURE_TEST
/// macro is used to specify what the test actually does.  Because it is
/// providing the implementation for a TextFixture method it needs to be called
/// before the `main()` function is created.
class TestFixture {
public:
    TestFixture(const std::string &name, const std::string &desc) :
        console{name},
        output_folder{name},
        _app{desc} {}

    int Run(int nargs, char **args) {
        std::optional<int> seed;
        _app.add_option("-s,--seed", seed, "Set the seed used by the feature test.");
        CLI11_PARSE(_app, nargs, args);

        console.Print(seed ? fmt::format("Seed: {}", *seed) : "Seed: N/A");
        console.Separator();

        Prng<> prng{seed ? *seed : PrngGenerator<>::DrawRandomSeed()};
        OperationTiming timer;
        try {
            Profile profile{timer};
            Func(prng);
        } catch (const errors::AbstractionsError &) {
            return 1;
        }

        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(timer.GetTiming().total);

        console.Separator();
        console.Print("Completed in {}", fmt::styled(time, fmt::emphasis::bold));

        return 0;
    }

    TestFixture(const TestFixture &) = delete;
    TestFixture(TestFixture &&) = delete;
    void operator=(const TestFixture &) = delete;
    void operator=(TestFixture &&) = delete;

protected:
    void Func(Prng<> &prng) const;

    Console console;
    TestOutputFolder output_folder;

private:
    CLI::App _app;
};

}  // namespace abstractions::tests
