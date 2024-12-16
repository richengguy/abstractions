#pragma once

#include <blend2d.h>

#include <filesystem>

namespace abstractions {

struct SurfaceBuffer {
    BLImage surface;

    SurfaceBuffer() = default;
    SurfaceBuffer(const BLImage &other);
    SurfaceBuffer(const SurfaceBuffer &other);
    SurfaceBuffer(const std::filesystem::path &path);
};

}  // namespace abstractions
