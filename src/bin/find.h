#pragma once

#include <abstractions/abstractions.h>

#include <filesystem>
#include <optional>

#include "support.h"

class FindCommand : public Command {
public:
    CLI::App *Init(CLI::App &parent) override;
    void Run() const override;

private:
    std::filesystem::path _image;
    abstractions::EngineConfig _config;
    abstractions::PgpeOptimizerSettings _optim_settings;
};
