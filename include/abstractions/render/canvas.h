#pragma once

#include <abstractions/image.h>
#include <abstractions/math/random.h>
#include <abstractions/types.h>
#include <blend2d.h>

#include <filesystem>

namespace abstractions {

/// @brief Compositing modes used by the renderer.
enum class CompositeMode {
    SrcCopy,
    SrcOver,
};

/// @brief A drawing surface for geometric shapes.
///
/// The canvas uses a scaled, anisotropic coordinate system such that the
/// height of the render surface is `[0, 1]` while the width is `[0, aspect]`,
/// with `aspect` being the surface aspect ratio. 
///
/// The Canvas is designed as an RAII wrapper around an existing Image.  All
/// draw operations are finalized whenever the Canvas goes out of scope, e.g.,
///
/// ```cpp
/// Image image;
/// {
///     Canvas canvas{image};
///     canvas.DrawFiledCircles(params);
/// }
/// image.Save("image.jpg");
/// ```
class Canvas {
public:
    /// @brief Create a new Canvas from an image.
    /// @param image image the canvas draws to
    /// @param seed PRNG seed used for any random draw operations; it will be a
    ///     random value if not provided
    Canvas(Image &image, std::optional<DefaultRngType::result_type> seed = {});

    /// @brief Create a new Canvas from an image.
    /// @param image image the canvas draws to
    /// @param prng PRNG used for all random draw operations
    Canvas(Image &image, Prng<DefaultRngType> &prng);

    /// @brief Clean up the rendering canvas when it is destroyed.
    ///
    /// The destructor cleans up the rendering canvas and ensures that any draw
    /// operations are flushed to the render surface.
    ~Canvas();

    /// @brief Set all pixels in the rendering surface to `(0,0,0,1)`.
    void Clear();

    /// @brief Set all pixels to the given colour.
    /// @param red red value
    /// @param green green value
    /// @param blue blue value
    /// @param alpha alpha value
    void Clear(const double red, const double green, const double blue, const double alpha = 1.0);

    /// @brief Draw a set of filled circles.
    /// @param params circles and their colours represented as an `Nx7` matrix
    /// @return an Error if the inupt dimensions are incorrect
    ///
    /// Each circle is represented as a `(x,y,s,r,g,b.a)` tuple, packed into a
    /// `Nx7` matrix.  The 's' dimension is a "signed radius", so it may be
    /// positive or negative.  The renderer will take the absolute value of 's'
    /// before doing any rendering.
    Error DrawFilledCircles(ConstMatrixRef params);

    /// @brief Draw a set of filled rectangles.
    /// @param params rectangles and their colours represeted as an `Nx8` matrix
    /// @return an Error if the inupt dimensions are incorrect
    ///
    /// Each rectangle is represented as a `(x1,y1,x2,y2,r,g,b,a)` tuple packed
    /// into a `Nx8` matrix.  `(x1,y1)` and `(x2,y2)` are the two corners of the
    /// rectangle.  The width is `|x1 - x2|` and the height is `|y1 - y2|`.
    Error DrawFilledRectangles(ConstMatrixRef params);

    /// @brief Draw a set of filled triangles.
    /// @param params triangles and their colours presented as an `Nx10` matrix
    /// @return an Error if the inupt dimensions are incorrect
    ///
    /// Each triangle is represented as a `(x1,y1,x2,y2,x3,y3,r,g,b,a)` tuple,
    /// packed into a `Nx10` matrix.
    Error DrawFilledTriangles(ConstMatrixRef params);

    /// @brief Fill the canvas with uniformly random values.
    /// @note The random numbers are generated from the PRNG that's passed into
    ///     the canvas when it's first created.
    void RandomFill();

    /// @brief Set the canvas' compositing mode.
    /// @param mode compositing mode
    /// @return an Error if the compositing mode is unsupported
    Error SetCompositeMode(const CompositeMode mode);

private:
    BLContext _context;
    Prng<DefaultRngType> _prng;
};

}  // namespace abstractions
