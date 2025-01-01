#pragma once

#include <doctest/doctest.h>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <random>
#include <string>

#include "test-paths.h"

/// @brief Used to parameterize unit tests.
///
/// See
/// https://github.com/doctest/doctest/blob/master/doc/markdown/parameterized-tests.md#value-parameterized-test-cases
/// for details on how this works.
#define ABSTRACTIONS_PARAMETERIZED_TEST(data, container)                                 \
    static size_t _subtest_id = 0;                                                       \
    std::for_each(std::begin(container), std::end(container), [&](const auto &element) { \
        std::string _test_name = fmt::format("{} - [{}]", #container, _subtest_id);      \
        _subtest_id++;                                                                   \
        DOCTEST_SUBCASE(_test_name.c_str()) {                                            \
            INFO("Test Value: ", element);                                               \
            data = element;                                                              \
        }                                                                                \
    });                                                                                  \
    _subtest_id = 0

namespace abstractions::tests {

/// @brief Use this at the top of a test to create a temporary folder.  The
///     default behaviour is to automatically clean up the folder when the test
///     completes.
class TempFolder {
public:
    TempFolder(bool cleanup = true) :
        _cleanup{cleanup},
        _folder{GenerateFolderName(doctest::getContextOptions())} {
        std::filesystem::create_directories(_folder);
    }

    ~TempFolder() {
        if (_cleanup) {
            std::filesystem::remove_all(_folder);
        }
    }

    std::filesystem::path Path() const {
        return _folder;
    }

    TempFolder(const TempFolder &) = delete;
    TempFolder(TempFolder &&) = delete;
    void operator=(const TempFolder &) = delete;
    void operator=(TempFolder &&) = delete;

private:
    static std::filesystem::path GenerateFolderName(const doctest::ContextOptions *options) {
        // Seems like this is the best way to get the current test name
        // https://github.com/doctest/doctest/issues/345
        auto context = doctest::getContextOptions();
        auto suffix = RandomName();

        if (!context) {
            return suffix;
        }

        if (!context->currentTest) {
            return suffix;
        }

        auto test_name = std::string(context->currentTest->m_name);
        std::string folder_name;

        for (int i = 0; i < test_name.size(); i++) {
            auto chr = test_name[i];

            if (std::ispunct(chr)) {
                continue;
            }

            if (std::isspace(chr)) {
                chr = '_';
            }

            folder_name += chr;
        }

        folder_name = fmt::format("{}-{}", folder_name, suffix);
        std::filesystem::path test_folder = kResultsPath / "t" / folder_name;

        return test_folder;
    }

    template <int length = 8>
    static std::string RandomName() {
        // NOTE: Adapted from https://stackoverflow.com/a/440240
        static const std::string chars =
            "0123456789"
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        std::random_device device;
        std::minstd_rand prng{device()};
        std::uniform_int_distribution<> rand(0, chars.size() - 1);

        std::vector<char> name(length);
        for (int i = 0; i < length; i++) {
            auto index = rand(prng);
            name[i] = chars[index];
        }

        return std::string(std::begin(name), std::end(name));
    }

    const bool _cleanup;
    const std::filesystem::path _folder;
};

}  // namespace abstractions::tests
