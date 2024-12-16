#pragma once

#include <filesystem>

#include <blend2d.h>

namespace abstractions
{

struct SurfaceBuffer
{
    BLImage surface;

    SurfaceBuffer() = default;
    SurfaceBuffer(const BLImage &other);
    SurfaceBuffer(const SurfaceBuffer &other);
    SurfaceBuffer(const std::filesystem::path &path);

};

}
