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

}

void RenderContext::DrawFilledTriangles(ConstMatrixRef params)
{

}

void RenderContext::DrawFillRectangles(ConstMatrixRef params)
{

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
