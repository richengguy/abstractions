#include "abstractions/image.h"

#include <abstractions/errors.h>
#include <fmt/format.h>
#include <fmt/std.h>

namespace abstractions {

namespace {

struct PixelDiff {
    const double red;
    const double green;
    const double blue;

    PixelDiff(uint32_t a, uint32_t b) :
        red{(detail::GetRedValue(a) - detail::GetRedValue(b)) / 255.0},
        green{(detail::GetGreenValue(a) - detail::GetGreenValue(b)) / 255.0},
        blue{(detail::GetBlueValue(a) - detail::GetBlueValue(b)) / 255.0} {}
};

template <typename AccFn>
Expected<double> PixelwiseComparison(const Expected<Image> &first, const Expected<Image> &second,
                                     AccFn fn) {
    abstractions_check(first);
    abstractions_check(second);

    const bool same_width = first->Width() == second->Width();
    const bool same_height = first->Height() == second->Height();

    if (!(same_width && same_height)) {
        return errors::report<double>(
            fmt::format("Cannot compare images; first image is {}x{} and second is {}x{}.",
                        first->Width(), first->Height(), second->Width(), second->Height()));
    }

    const int width = first->Width();
    const int height = first->Height();

    PixelData ref = first->Pixels();
    PixelData tgt = second->Pixels();

    double sum = 0;
    for (int y = 0; y < height; y++) {
        auto row_ref = ref.Row(y);
        auto row_tgt = tgt.Row(y);

        for (int x = 0; x < width; x++) {
            PixelDiff diff(row_ref[x], row_tgt[x]);
            sum += fn(diff);
        }
    }

    return sum / (width * height);
}

}  // namespace

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
        return errors::report<Image>(fmt::format("Failed to load {} (Error Code: {})", path, err));
    }

    auto format = buffer.format();
    if (format != BL_FORMAT_XRGB32 && format != BL_FORMAT_PRGB32) {
        return errors::report<Image>("Image must be 32-bit RGB or RGBA.");
    }

    return Image(buffer);
}

Expected<Image> Image::New(const int width, const int height, const bool with_alpha) {
    if (width <= 0 || height <= 0) {
        return errors::report<Image>(fmt::format(
            "The width and height cannot less than zero (width: {}, height: {})", width, height));
    }

    auto format = with_alpha ? BL_FORMAT_PRGB32 : BL_FORMAT_XRGB32;
    BLImage buffer(width, height, format);
    abstractions_assert(!buffer.empty());
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

PixelFormat Image::Format() const {
    switch (_buffer.format()) {
        case BL_FORMAT_PRGB32:
            return PixelFormat::RGBWithPremultAlpha;
        case BL_FORMAT_XRGB32:
            return PixelFormat::RGB;
        default:
            return PixelFormat::Unknown;
    }
}

PixelData Image::Pixels() const {
    BLImageData surface;
    _buffer.getData(&surface);
    return PixelData(surface);
}

Error Image::ScaleToFit(int dim) {
    if (dim < 8) {
        return "Cannot scale smaller than 8x8.";
    }

    // Get the size of the longest dimension.  If it's already less than the
    // maximum dimension size, do nothing.
    int longest_size = Width() > Height() ? Width() : Height();
    if (longest_size <= dim) {
        return errors::no_error;
    }

    // Compute the new size that will fit into the specified dimensions.
    double scale = static_cast<double>(dim) / longest_size;
    int scaled_width = std::round(scale * Width());
    int scaled_height = std::round(scale * Height());

    BLSizeI scaled_size(scaled_width, scaled_height);
    BLImage scaled_image;
    auto err = BLImage::scale(scaled_image, _buffer, scaled_size,
                              BLImageScaleFilter::BL_IMAGE_SCALE_FILTER_LANCZOS);
    if (err != BL_SUCCESS) {
        return fmt::format("Could resize image (error {})", err);
    }

    _buffer = scaled_image;
    return errors::no_error;
}

Error Image::Save(const std::filesystem::path &path) const {
    BLResult err;
    err = _buffer.writeToFile(path.c_str());
    if (err != BL_SUCCESS) {
        return fmt::format("Could not write to '{}' (error {})", path, err);
    }

    return errors::no_error;
}

Expected<double> CompareImagesAbsDiff(const Expected<Image> &first, const Expected<Image> &second) {
    return PixelwiseComparison(first, second, [](PixelDiff &diff) {
        return std::abs(diff.red) + std::abs(diff.green) + std::abs(diff.blue);
    });
}

Expected<double> CompareImagesSquaredDiff(const Expected<Image> &first,
                                          const Expected<Image> &second) {
    return PixelwiseComparison(first, second, [](PixelDiff &diff) {
        return diff.red * diff.red + diff.green * diff.green + diff.blue * diff.blue;
    });
}

}  // namespace abstractions
