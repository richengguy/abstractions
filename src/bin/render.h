#pragma once

#include "support.h"

class RenderCommand : public ICommand
{
public:
    CLI::App *Init(CLI::App &parent) override;
    void Run() const override;
private:

};
