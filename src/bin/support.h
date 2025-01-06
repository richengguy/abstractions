#pragma once

#include <CLI/CLI.hpp>

#include <filesystem>
#include <type_traits>

// Custom type names used in CLI11

namespace CLI::detail
{

template<>
constexpr const char *type_name<std::filesystem::path>()
{
    return "PATH";
}

}

/// @brief Standard format for a CLI command.
struct ICommand
{
    virtual CLI::App *Init(CLI::App &parent) = 0;
    virtual void Run() const = 0;
    virtual ~ICommand() = default;
};

/// @brief Register a CLI command with the parent app.
/// @tparam T command type
/// @param parent parent app
/// @param command command being registered
template<typename T>
void Register(CLI::App &parent, T &command)
{
    static_assert(std::is_base_of_v<ICommand, T>, "'T' must implement ICommand.");
    auto subcmd = command.Init(parent);
    subcmd->callback([&command](){ command.Run(); });
}
