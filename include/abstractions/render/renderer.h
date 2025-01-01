#pragma once

#include <optional>

#include <abstractions/image.h>
#include <abstractions/math/random.h>
#include <abstractions/render/shapes.h>

namespace abstractions::render
{

/// @brief Renders an abstract image from a shape collection.
class Renderer
{
public:
    /// @brief Create a new renderer with the given canvas size.
    /// @param width canvas width
    /// @param height canvas height
    /// @param seed random seed (used for some draw operations)
    static Expected<Renderer> Create(int width, int height, std::optional<DefaultRngType::result_type> seed = {});

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
    void Draw(const PackedShapeCollection &shapes);

private:
    Renderer(Image &image, std::optional<DefaultRngType::result_type> seed);
    bool _random_background;
    Pixel _background_colour;
    Image _drawing_surface;
};

}
