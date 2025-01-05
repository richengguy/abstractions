#pragma once

#include "command.h"

#include <optional>
#include <filesystem>

class FindCommand : public ICommand
{
public:
    CLI::App *Init(CLI::App &parent) override;
    void Run() const override;

private:
    std::filesystem::path _image;
    std::optional<int> _seed;
};
