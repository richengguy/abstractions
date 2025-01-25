#pragma once

#include <abstractions/terminal/console.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <CLI/CLI.hpp>
#include <filesystem>
#include <initializer_list>
#include <string_view>
#include <tuple>
#include <type_traits>

#ifdef ABSTRACTIONS_ENABLE_GPERFTOOLS
#include <gperftools/profiler.h>
#endif // ABSTRACTIONS_ENABLE_GPERFTOOLS

// Custom type names used in CLI11

namespace CLI::detail {

template <>
constexpr const char *type_name<std::filesystem::path>() {
    return "PATH";
}

}  // namespace CLI::detail

// Other CLI1 library helpers

namespace cli_helpers {

/// @brief A simple "convert string to lower case" function.
/// @param[inout] str string to convert
void to_lower(std::string &str);

/// @brief A simple "convert string to lower case" function.
/// @param str string to convert
/// @return converted string
std::string to_lower(std::string_view str);

template <typename T>
struct EnumValidator : public ::CLI::Validator {
    EnumValidator(const std::string &name, std::initializer_list<T> values) {
        name_ = name;

        std::vector<T> enum_values(std::begin(values), std::end(values));
        desc_function_ = [enum_values]() {
            return fmt::format("({})", fmt::join(enum_values, ", "));
        };

        std::vector<std::tuple<T, std::string>> idents;
        for (auto &v : enum_values) {
            idents.push_back(std::make_tuple(v, to_lower(fmt::format("{}", v))));
        }

        func_ = [idents, enum_values](std::string &input) {
            bool has_value = false;
            T value;
            for (const auto &[v, i] : idents) {
                if (i == input) {
                    value = v;
                    has_value = true;
                    break;
                }
            }

            if (!has_value) {
                return fmt::format("{} is not one of \"{}\".", input, fmt::join(enum_values, ", "));
            }

            input = CLI::detail::value_string(value);
            return std::string{};
        };
    }
};

}  // namespace cli_helpers

/// @brief Standard format for a CLI command.
class Command {
public:
    Command();
    virtual CLI::App *Init(CLI::App &parent) = 0;
    virtual void Run() const = 0;
    virtual ~Command() = default;

protected:
    abstractions::terminal::Console console;
};

/// @brief Register a CLI command with the parent app.
/// @tparam T command type
/// @param parent parent app
/// @param command command being registered
template <typename T>
void Register(CLI::App &parent, T &command) {
    static_assert(std::is_base_of_v<Command, T>, "'T' must implement ICommand.");
    auto subcmd = command.Init(parent);
    subcmd->callback([&command]() {
        #ifdef ABSTRACTIONS_ENABLE_GPERFTOOLS
        fmt::println("⚠️ Profiling enabled; saving to 'abstractions.profile'.");
        ProfilerStart("abstractions.profile");
        #endif // ABSTRACTIONS_ENABLE_GPERFTOOLS

        command.Run();

        #ifdef ABSTRACTIONS_ENABLE_GPERFTOOLS
        ProfilerStop();
        #endif // ABSTRACTIONS_ENABLE_GPERFTOOLS
    });
}
