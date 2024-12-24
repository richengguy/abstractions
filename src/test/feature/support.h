#pragma once

#include <abstractions/math/random.h>
#include <abstractions/profile.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>

#include <CLI/CLI.hpp>
#include <filesystem>
#include <optional>

#include "test-paths.h"

/// @brief Creates the objects needed for a feature test.
/// @param test_name the feature test name
///
/// Two objects will be created in the local scope.  `console` is used to print
/// messages to the console in a standard format.  `test_folder` is an object
/// that represents the local working space for the test.  It will clean the
/// folder contents before the test runs.
#define ABSTRACTIONS_INIT_FEATURE_TEST(test_name)                   \
    ::abstractions::tests::Console console(test_name);              \
    console.Print("Initializing feature test.");                    \
    ::abstractions::tests::TestOutputFolder test_folder(test_name); \
    console.Separator()

/// @brief Use this to specify what the feature test actually does.
///
/// The feature test will have access to the following objects:
///
/// * `console` - Method for providing logging output.
/// * `output_folder` - An object that provides some scratch space for the test.
/// * `prng` - A PRNG instance the test can use.
#define ABSTRACTIONS_FEATURE_TEST() void ::abstractions::tests::TestFixture::Func() const

/// @brief Creates the "main()" function for a feature test in a standardized way.
/// @param name test name
/// @param desc short description shown in the commandline
#define ABSTRACTIONS_FEATURE_TEST_MAIN(name, desc)              \
    int main(int nargs, char **args) {                          \
        ::abstractions::tests::TestFixture fixture(name, desc); \
        return fixture.Run(nargs, args);                        \
    }

namespace abstractions::tests {

/// @brief Simple logging utility for printing commonly-formatting messages to a console.
class Console {
public:
    Console(const std::string &test_name) {
        _test_name =
            fmt::format("{}", fmt::styled(test_name, fmt::emphasis::faint | fmt::emphasis::italic));
    }

    void Separator(int length = 10) const {
        fmt::println("{:\u2500^{}}", "", length);
    }

    void Print(const std::string &msg) const {
        fmt::println("{} :: {}", _test_name, msg);
    }

private:
    std::string _test_name;
};

/// @brief Manages the output results folder for a particular feature test.
class TestOutputFolder {
public:
    TestOutputFolder(const std::string &test_name) :
        _folder{kResultsPath / "s" / test_name} {
        Console console(test_name);

        if (std::filesystem::exists(_folder)) {
            auto removed = std::filesystem::remove_all(_folder);
            console.Print(
                fmt::format("Removed {} files.", fmt::styled(removed, fmt::emphasis::bold)));
        }

        auto ret = std::filesystem::create_directories(_folder);
        if (ret) {
            console.Print(
                fmt::format("Created {}", fmt::styled(_folder.c_str(), fmt::emphasis::bold)));
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
        prng{0},
        _app{desc} {}

    int Run(int nargs, char **args) {
        std::optional<int> seed;
        _app.add_option("-s,--seed", seed, "Set the seed used by the feature test.");
        CLI11_PARSE(_app, nargs, args);

        console.Print(seed ? fmt::format("Seed: {}", *seed) : "Seed: N/A");
        prng = Prng<>(seed ? *seed : PrngGenerator<>::DrawRandomSeed());
        console.Separator();

        OperationTiming timer;
        try {
            Profile profile{timer};
            Func();
        } catch (const errors::AbstractionsError &) {
            return 1;
        }

        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(timer.GetTiming().total);

        console.Separator();
        console.Print(fmt::format("Completed in {}", fmt::styled(time, fmt::emphasis::bold)));

        return 0;
    }

    TestFixture(const TestFixture &) = delete;
    TestFixture(TestFixture &&) = delete;
    void operator=(const TestFixture &) = delete;
    void operator=(TestFixture &&) = delete;

protected:
    void Func() const;

    Console console;
    TestOutputFolder output_folder;
    Prng<> prng;

private:
    CLI::App _app;
};

}  // namespace abstractions::tests
