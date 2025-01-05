#pragma once

#include <CLI/CLI.hpp>

#include <type_traits>

struct ICommand
{
    virtual CLI::App *Init(CLI::App &parent) = 0;
    virtual void Run() const = 0;
    virtual ~ICommand() = default;
};

template<typename T>
void Register(CLI::App &parent, T &command)
{
    static_assert(std::is_base_of_v<ICommand, T>, "'T' must implement ICommand.");
    auto subcmd = command.Init(parent);
    subcmd->callback([&command](){ command.Run(); });
}
