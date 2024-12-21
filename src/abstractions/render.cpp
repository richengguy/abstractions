#include <abstractions/render.h>

#include <abstractions/errors.h>

#include <fmt/format.h>

namespace abstractions
{

RenderContext::RenderContext(Image &image)
    : _context{image}
{

}

void RenderContext::Clear()
{
    Clear(0, 0, 0, 1);
}

void RenderContext::Clear(const double red, const double green, const double blue, const double alpha)
{
    BLRgba colour(red,green,blue,alpha);
    auto orig_mode = _context.compOp();
    _context.setCompOp(BL_COMP_OP_SRC_OVER);
    _context.setFillStyle(colour);
    _context.fillAll();
    _context.setCompOp(orig_mode);
}

void RenderContext::DrawFilledCircles(ConstMatrixRef params)
{
    const int num_circles = params.rows();
    const int num_dimensions = params.cols();
    abstractions_assert(num_dimensions == 7);

    // Scaling is anisotropic, so vertical is [0,1] while horizontal is
    // [0, aspect].  This means getting to the full size image is just a matter
    // of multiplying by the height.

    const double scale = _context.targetHeight() - 1;

    for (int i = 0; i < num_circles; i++)
    {
        const RowVector row = params.row(i);

        const BLRgba colour(row[3], row[4], row[5], row[6]);
        const BLCircle circle(
            scale * row[0],
            scale * row[1],
            scale * std::abs(row[2])
        );
        _context.fillCircle(circle, colour);
    }
}

void RenderContext::DrawFilledTriangles(ConstMatrixRef params)
{
    const int num_triangles = params.rows();
    const int num_dimensions = params.cols();
    abstractions_assert(num_dimensions == 10);

    // Scaling is anisotropic, so vertical is [0,1] while horizontal is
    // [0, aspect].  This means getting to the full size image is just a matter
    // of multiplying by the height.

    const double scale = _context.targetHeight() - 1;

    for (int i = 0; i < num_triangles; i++)
    {
        const RowVector row = params.row(i);

        const BLRgba colour(row[6], row[7], row[8], row[9]);
        const BLTriangle triangle(
            scale * row[0], scale * row[1],
            scale * row[2], scale * row[3],
            scale * row[4], scale * row[5]
        );
        _context.fillTriangle(triangle, colour);
    }
}

void RenderContext::DrawFilledRectangles(ConstMatrixRef params)
{
    const int num_rects = params.rows();
    const int num_dimensions = params.cols();
    abstractions_assert(num_dimensions == 8);

    // Scaling is anisotropic, so vertical is [0,1] while horizontal is
    // [0, aspect].  This means getting to the full size image is just a matter
    // of multiplying by the height.

    const double scale = _context.targetHeight() - 1;

    for (int i = 0; i < num_rects; i++)
    {
        const RowVector row = params.row(i);

        const double x1 = scale * row[0];
        const double y1 = scale * row[1];
        const double x2 = scale * row[2];
        const double y2 = scale * row[3];

        const double x = std::min(x1, x2);
        const double y = std::min(y1, y2);
        const double w = std::abs(x1 - x2);
        const double h = std::abs(y1 - y2);

        const BLRgba colour(row[4], row[5], row[6], row[7]);
        const BLRect rect(x, y, w, h);
        _context.fillRect(rect, colour);
    }
}

Error RenderContext::SetCompositeMode(const CompositeMode mode)
{
    BLCompOp op;
    switch (mode)
    {
        case kCompositeModeSrcCopy:
            op = BL_COMP_OP_SRC_COPY;
            break;
        case kCompositeModeSrcOver:
            op = BL_COMP_OP_SRC_OVER;
            break;
        default:
            return Error(fmt::format("Unknown compositing mode {}.", static_cast<int>(mode)));
    }

    _context.setCompOp(op);

    return errors::no_error;
}

}
