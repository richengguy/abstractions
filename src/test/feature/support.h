#pragma once

#include <fmt/color.h>
#include <fmt/format.h>

#include <filesystem>

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

    std::filesystem::path Path() const {
        return _folder;
    }

    TestOutputFolder(const TestOutputFolder &) = delete;
    TestOutputFolder(TestOutputFolder &&) = delete;
    void operator=(const TestOutputFolder &) = delete;
    void operator=(TestOutputFolder &&) = delete;

private:
    const std::filesystem::path _folder;
};

}  // namespace abstractions::tests
