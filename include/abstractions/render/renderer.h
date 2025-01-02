#pragma once

#include <optional>

#include <abstractions/image.h>
#include <abstractions/math/random.h>
#include <abstractions/render/shapes.h>

namespace abstractions::render
{

/// @brief Renders an abstract image from a shape collection.
///
/// The renderers maintains an internal rendering surface and can be reused.
/// Each call to Renderer::Render() will clear out the surface before rendering
/// the provided shapes.
class Renderer
{
public:
    /// @brief Create a new renderer with the given canvas size.
    /// @param width canvas width
    /// @param height canvas height
    /// @param prng a PRNG instance (used for some draw operations)
    static Expected<Renderer> Create(int width, int height, std::optional<Prng<>> prng = {});

    /// @brief Enables/disables the use of a randomized background fill.
    /// @param use_random enable/disable randomized background file
    ///
    /// This will tell the renderer to fill in the background with random pixel
    /// values rather than using a set background colour.
    void UseRandomBackgroundFill(bool use_random);

    /// @brief Set the background fill colour.  This is ignored when the random
    ///     background fill is enabled.
    /// @param red red value
    /// @param green green value
    /// @param blue blue value
    /// @param alpha alpha value; defaults to "fully opaque"
    void SetBackground(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);

    /// @brief Draw the packed collection.
    /// @param shapes set of shapes for the renderer to draw
    /// @return the rendering result
    void Render(const PackedShapeCollection &shapes);

    /// @brief Read-only access to the internal drawing surface
    const Image &DrawingSurface() const {
        return _drawing_surface;
    }

private:
    Renderer(Image &image, std::optional<Prng<>> seed);
    Prng<> _prng;
    bool _random_background;
    Pixel _background_colour;
    Image _drawing_surface;
};

}
