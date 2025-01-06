#include "abstractions/render/renderer.h"

#include <abstractions/math/random.h>
#include <abstractions/render/canvas.h>

namespace abstractions::render {

Expected<Renderer> Renderer::Create(int width, int height, std::optional<Prng<>> prng) {
    auto image = Image::New(width, height, true);
    if (image.has_value()) {
        return Renderer(*image, prng);
    }
    return errors::report<Renderer>(image.error());
}

Renderer::Renderer(Image &image, std::optional<Prng<>> prng) :
    _prng{prng.value_or(Prng<>(PrngGenerator<>::DrawRandomSeed()))},
    _random_background{false},
    _background_colour{0xff, 0xff, 0xff},
    _drawing_surface{image} {}

void Renderer::UseRandomBackgroundFill(bool use_random) {
    _random_background = use_random;
}

void Renderer::SetBackground(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
    _background_colour = Pixel(red, green, blue, alpha);
}

void Renderer::SetBackground(const Pixel &background)
{
    _background_colour = background;
}

void Renderer::Render(const PackedShapeCollection &shapes) {
    Canvas canvas{_drawing_surface, _prng};

    if (_random_background) {
        canvas.RandomFill();
    } else {
        double r = static_cast<double>(_background_colour.Red()) / 255.0;
        double g = static_cast<double>(_background_colour.Green()) / 255.0;
        double b = static_cast<double>(_background_colour.Blue()) / 255.0;
        double a = static_cast<double>(_background_colour.Alpha()) / 255.0;
        canvas.Clear(r, g, b, a);
    }

    auto selected_shapes = shapes.Shapes();
    if (selected_shapes & AbstractionShape::Circles) {
        canvas.DrawFilledCircles(shapes.Circles().Params);
    }

    if (selected_shapes & AbstractionShape::Rectangles) {
        canvas.DrawFilledRectangles(shapes.Rectangles().Params);
    }

    if (selected_shapes & AbstractionShape::Triangles) {
        canvas.DrawFilledTriangles(shapes.Triangles().Params);
    }
}

}  // namespace abstractions::render
