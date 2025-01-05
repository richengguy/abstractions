#pragma once

#include "command.h"

class RenderCommand : public ICommand
{
public:
    CLI::App *Init(CLI::App &parent) override;
    void Run() const override;
private:

};
