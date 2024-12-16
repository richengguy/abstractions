#include <abstractions/errors.h>
#include <abstractions/image.h>
#include <fmt/format.h>
#include <fmt/std.h>

namespace abstractions {

PixelData::PixelData(const BLImageData &surface) :
    _surface{surface} {}

const uint32_t *PixelData::Row(const int y) const {
    abstractions_assert(y >= 0 && y < _surface.size.h);

    const uint8_t *buffer = static_cast<const uint8_t *>(_surface.pixelData);
    return reinterpret_cast<const uint32_t *>(&buffer[y * _surface.stride]);
}

Pixel PixelData::Get(const int x, const int y) const {
    abstractions_assert(x >= 0 && x < _surface.size.w);
    abstractions_assert(y >= 0 && y < _surface.size.h);

    const uint8_t *buffer = static_cast<const uint8_t *>(_surface.pixelData);
    const uint32_t *row = reinterpret_cast<const uint32_t *>(&buffer[y * _surface.stride]);

    return Pixel(row[x]);
}

int PixelData::Width() const {
    return _surface.size.w;
}

int PixelData::Height() const {
    return _surface.size.h;
}

intptr_t PixelData::Stride() const {
    return _surface.stride;
}

Expected<Image> Image::Load(const std::filesystem::path &path) {
    BLImage buffer;
    auto err = buffer.readFromFile(path.c_str());
    if (err != BL_SUCCESS) {
        return errors::report<Image>(fmt::format("Failed to load {} ({})", path, err));
    }

    auto format = buffer.format();
    if (format != BL_FORMAT_XRGB32 && format != BL_FORMAT_PRGB32) {
        return errors::report<Image>("Image must be 32-bit RGB or RGBA.");
    }

    return Image(buffer);
}

Image::Image(const BLImage &buffer) :
    _buffer{buffer} {}

int Image::Width() const {
    return _buffer.width();
}

int Image::Height() const {
    return _buffer.height();
}

int Image::Depth() const {
    return _buffer.depth();
}

PixelData Image::Pixels() const {
    BLImageData surface;
    _buffer.getData(&surface);
    return PixelData(surface);
}

}  // namespace abstractions
