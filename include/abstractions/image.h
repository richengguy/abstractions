#pragma once

#include <abstractions/types.h>
#include <blend2d.h>

#include <filesystem>
#include <memory>

namespace abstractions {

namespace detail {

/// @brief Get an individual pixel component in a packed 32-bit pixel.
/// @tparam N component channel
/// @param pixel a packed, e.g., RGBA, 32-bit pixel value
/// @return the indiviual 8-bit component value
template <int N>
constexpr uint8_t GetComponent(const uint32_t pixel) {
    static_assert(N >= 0 && N <= 3, "Offset must be 0, 1, 2, or 3.");
    return (pixel >> (N << 3)) & 0xff;
}

/// @brief Set an individual pixel component in a packed 32-bit pixel.
/// @tparam N component channel
/// @param value 8-bit value to set
/// @param pixel the packed, e.g., RGBA, 32-bit pixel value
template <int N>
constexpr void SetComponent(const uint8_t value, uint32_t &pixel) {
    static_assert(N >= 0 && N <= 3, "Offset must be 0, 1, 2, or 3.");
    const uint32_t mask = 0x000000ff << (N << 3);
    pixel &= ~mask;
    pixel |= (value << (N << 3)) & mask;
}

constexpr uint8_t GetAlphaValue(const uint32_t pixel) {
    return GetComponent<3>(pixel);
}
constexpr uint8_t GetRedValue(const uint32_t pixel) {
    return GetComponent<2>(pixel);
}
constexpr uint8_t GetGreenValue(const uint32_t pixel) {
    return GetComponent<1>(pixel);
}
constexpr uint8_t GetBlueValue(const uint32_t pixel) {
    return GetComponent<0>(pixel);
}

constexpr void SetAlphaValue(const uint8_t alpha, uint32_t &pixel) {
    SetComponent<3>(alpha, pixel);
}
constexpr void SetRedValue(const uint8_t red, uint32_t &pixel) {
    SetComponent<2>(red, pixel);
}
constexpr void SetGreenValue(const uint8_t green, uint32_t &pixel) {
    SetComponent<1>(green, pixel);
}
constexpr void SetBlueValue(const uint8_t blue, uint32_t &pixel) {
    SetComponent<0>(blue, pixel);
}

/// @brief Pack 8-bit colour values into a single packed ABGR pixel.
/// @param r red value
/// @param g green value
/// @param b blue value
/// @param a alpha value
/// @return packed value
constexpr uint32_t PackComponents(const uint8_t r, const uint8_t g, const uint8_t b,
                                  const uint8_t a) {
    uint32_t pixel = 0;
    SetRedValue(r, pixel);
    SetGreenValue(g, pixel);
    SetBlueValue(b, pixel);
    SetAlphaValue(a, pixel);
    return pixel;
}

}  // namespace detail

/// @brief Provides easy access to the contents of a 32-bit ABGR pixel value.
///
/// The Pixel class is interchangeable with a `uint32_t`.  A Pixel instance can
/// be compared to an unsigned integer and vice-versa.  There is also suppport
/// for direct conversion.
class Pixel {
public:
    /// @brief Create a new pixel instance.
    /// @param pixel packed 32-bit pixel
    Pixel(const uint32_t pixel) :
        _pixel{pixel} {}

    /// @brief Create a new pixel instance
    /// @param r red value
    /// @param g green value
    /// @param b blue value
    /// @param a alpha value
    Pixel(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 0xff) :
        _pixel{detail::PackComponents(r, g, b, a)} {}

    Pixel(const Pixel &) = default;
    Pixel(Pixel &&) = default;
    Pixel &operator=(const Pixel &) = default;
    Pixel &operator=(Pixel &&) = default;

    bool operator==(const Pixel &other) const {
        return _pixel == other._pixel;
    }

    bool operator==(const uint32_t other) const {
        return _pixel == other;
    }

    operator uint32_t() const {
        return _pixel;
    }

    operator int() const {
        return _pixel;
    }

    uint8_t Red() const {
        return detail::GetRedValue(_pixel);
    }
    uint8_t Green() const {
        return detail::GetGreenValue(_pixel);
    }
    uint8_t Blue() const {
        return detail::GetBlueValue(_pixel);
    }
    uint8_t Alpha() const {
        return detail::GetAlphaValue(_pixel);
    }

private:
    uint32_t _pixel;
};

/// @brief Provides read-only access to the underlying Image pixel data.
class PixelData {
public:
    /// @brief Get a pointer to the start of an image row.
    /// @param y row's y-coordinate
    /// @return row pointer
    ///
    /// The row pointer will contain PixelData::Height() elements.  The
    /// individual pixel values can be extracted with something like,
    ///
    /// ```cpp
    /// auto row = pixels.Row(y);
    /// auto pixel = Pixel(row[x]);
    /// ```
    const uint32_t *Row(const int y) const;

    /// @brief Get a pixel at the given coordinate.
    /// @param x x-coordinate
    /// @param y y-coordinate
    /// @return pixel value
    Pixel Get(const int x, const int y) const;

    /// @brief Image width
    int Width() const;

    /// @brief Image height
    int Height() const;

    /// @brief The stride between image rows.
    intptr_t Stride() const;

private:
    friend class Image;

    PixelData(const BLImageData &surface);
    BLImageData _surface;
};

/// @brief A 32-bit, RGB image with an optional alpha channel.
///
/// The Image class is a thin wrapper on top of a Blend2D
/// [BLImage](https://blend2d.com/doc/classBLImage.html) instance.
class Image {
public:
    /// @brief Load an image from a file.
    /// @param path File path.
    /// @return The loaded image or an error if it failed to load.
    static Expected<Image> Load(const std::filesystem::path &path);

    /// @brief Create a new Image from an existing Blend2D `BLImage` instance.
    /// @param image a `BLImage` instance
    Image(const BLImage &image);

    /// @brief Image height, in pixels.
    int Height() const;

    /// @brief Image width, in pixels.
    int Width() const;

    /// @brief The total number of bits per pixel.
    /// @note This will either be 24 (RGB) or 32 (RGBA), with the internal
    ///     storage *always* being equivalent to `uint32_t`.
    int Depth() const;

    /// @brief Get read-only access to the underlying pixel data.
    /// @return An object for accessing the pixel data.
    PixelData Pixels() const;

    /// @brief Convert the image to its underlying `BLImage` buffer.
    operator BLImage &() {
        return _buffer;
    }

private:
    BLImage _buffer;
};

}  // namespace abstractions
