#pragma once

#include <abstractions/abstractions.h>

#include <filesystem>
#include <optional>

#include "support.h"

class GenerateCommand : public Command {
public:
    CLI::App *Init(CLI::App &parent) override;
    void Run() const override;

private:
    std::filesystem::path _image;
    std::filesystem::path _output;
    std::filesystem::path _per_stage_output;
    std::optional<double> _image_scale;
    abstractions::EngineConfig _config;
    abstractions::PgpeOptimizerSettings _optim_settings;
};
