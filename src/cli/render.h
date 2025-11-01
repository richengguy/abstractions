#pragma once

#include <filesystem>

#include "support.h"

class RenderCommand : public Command {
public:
    CLI::App *Init(CLI::App &parent) override;
    void Run() const override;

private:
    int _dim;
    bool _use_width;
    bool _use_height;
    std::filesystem::path _json;
    std::filesystem::path _output;
};
