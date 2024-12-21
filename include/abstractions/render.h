#pragma once

#include <abstractions/image.h>
#include <abstractions/types.h>

#include <blend2d.h>

#include <filesystem>

namespace abstractions {

/// @brief Compositing modes used by the renderer.
enum CompositeMode
{
    kCompositeModeSrcCopy,
    kCompositeModeSrcOver,
};

class RenderContext
{
public:

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
    ///
    /// Each circle is represented as a `(x,y,s,r,g,b.a)` tuple, packed into a
    /// `Nx7` matrix.  The 's' dimension is a "signed radius", so it may be
    /// positive or negative.  The renderer will take the absolute value of 's'
    /// before doing any rendering.
    void DrawFilledCircles(ConstMatrixRef params);

    /// @brief Draw a set of filled rectangles.
    /// @param params rectangles and their colours represeted as an `Nx8` matrix
    ///
    /// Each rectangle is represented as a `(x1,y1,x2,y2,r,g,b,a)` tuple packed
    /// into a `Nx8` matrix.  `(x1,y1)` is the bottom-left corner and `(x2,y2)`
    /// is the top-right corner.
    void DrawFillRectangles(ConstMatrixRef params);

    /// @brief Draw a set of filled triangles.
    /// @param params triangles and their colours presented as an `Nx10` matrix
    ///
    /// Each triangle is represented as a `(x1,y1,x2,y2,x3,y3,r,g,b,a)` tuple,
    /// packed into a `Nx10` matrix.
    void DrawFilledTriangles(ConstMatrixRef params);

    /// @brief Set the renderer's compositing mode.
    /// @param mode compositing mode
    /// @return an Error if the compositing mode is unsupported
    Error SetCompositeMode(const CompositeMode mode);

private:
    friend class Renderer;
    RenderContext(Image &image);
    BLContext _context;
};

class Renderer
{
public:
private:
};

}  // namespace abstractions
