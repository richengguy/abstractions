#include "abstractions/render/renderer.h"

#include <abstractions/math/random.h>

namespace abstractions::render
{

Expected<Renderer> Renderer::Create(int width, int height, std::optional<DefaultRngType::result_type> seed)
{
    auto image = Image::New(width, height, true);
    if (image.has_value())
    {
        return Renderer(image.value(), seed);
    }
    return errors::report<Renderer>(image.error());
}

Renderer::Renderer(Image &image, std::optional<DefaultRngType::result_type> seed)
    : _random_background{false},
      _background_colour{0xff, 0xff, 0xff},
      _drawing_surface{image}
{

}

}
