#include "abstractions/render/canvas.h"

#include <abstractions/errors.h>
#include <abstractions/math/matrices.h>
#include <fmt/format.h>

#include <algorithm>

namespace abstractions::render {

Canvas::Canvas(Expected<Image> &image, std::optional<DefaultRngType::result_type> seed) :
    _prng{seed.value_or(PrngGenerator<DefaultRngType>::DrawRandomSeed())},
    _alpha_scale{1.0} {
    abstractions_check(image);
    _context = BLContext(*image);
}

Canvas::Canvas(Expected<Image> &image, Prng<DefaultRngType> prng) :
    _prng{prng},
    _alpha_scale{1.0} {
    abstractions_check(image);
    _context = BLContext(*image);
}

Canvas::Canvas(Image &image, Prng<DefaultRngType> prng) :
    _context{BLContext(image)},
    _prng{prng},
    _alpha_scale{1.0} {}

Canvas::~Canvas() {
    _context.end();
}

void Canvas::Clear() {
    Clear(0, 0, 0, 1);
}

void Canvas::Clear(const double red, const double green, const double blue, const double alpha) {
    BLRgba colour(red, green, blue, alpha);
    auto orig_mode = _context.compOp();
    _context.setCompOp(BL_COMP_OP_SRC_OVER);
    _context.setFillStyle(colour);
    _context.fillAll();
    _context.setCompOp(orig_mode);
}

Error Canvas::DrawFilledCircles(ConstMatrixRef params) {
    const int num_circles = params.rows();
    const int num_dimensions = params.cols();
    if (num_dimensions != 7) {
        return Error(
            fmt::format("Expected a Nx7 array, got an {}x{}.", num_circles, num_dimensions));
    }

    // Scaling is isotropic, so vertical is [0,1] while horizontal is
    // [0, aspect].  This means getting to the full size image is just a matter
    // of multiplying by the height.

    const double x_scale = _context.targetWidth() - 1;
    const double y_scale = _context.targetHeight() - 1;
    const double r_scale = y_scale;

    // Force the shapes to be *mostly* inside of the frame but keep the colour
    // values clamped on [0, 1] since anything outside that doesn't make any
    // sense.  The circle radii also have to be constrained more than other
    // shapes as they can easily grow to cover the entire image.

    Matrix prepped = params;
    prepped.leftCols(2) = 1.2 * RescaleValuesColumnWise(params.leftCols(2)).array() - 0.1;

    prepped.rightCols(4) = params.rightCols(4);
    prepped.rightCols(1) *= _alpha_scale;
    prepped.rightCols(4) = ClampValues(prepped.rightCols(4));

    for (int i = 0; i < num_circles; i++) {
        const RowVector row = prepped.row(i);

        const BLRgba colour(row[3], row[4], row[5], row[6]);
        // clang-format off
        const BLCircle circle(
            x_scale * row[0],
            y_scale * row[1],
            r_scale * std::abs(row[2])
        );
        // clang-format on
        _context.fillCircle(circle, colour);
    }

    return errors::no_error;
}

Error Canvas::DrawFilledTriangles(ConstMatrixRef params) {
    const int num_triangles = params.rows();
    const int num_dimensions = params.cols();

    if (num_dimensions != 10) {
        return Error(
            fmt::format("Expected a Nx10 array, got an {}x{}.", num_triangles, num_dimensions));
    }

    // Scaling is isotropic, so vertical is [0,1] while horizontal is
    // [0, aspect].  This means getting to the full size image is just a matter
    // of multiplying by the height.

    const double x_scale = _context.targetWidth() - 1;
    const double y_scale = _context.targetHeight() - 1;

    // Force the shapes to be *mostly* inside of the frame but keep the colour
    // values clamped on [0, 1] since anything outside that doesn't make any
    // sense.  Apply the alpha scaling right before any clamping.

    Matrix prepped = params;
    prepped.leftCols(6) = 1.2 * RescaleValuesColumnWise(params.leftCols(6)).array() - 0.1;

    prepped.rightCols(4) = params.rightCols(4);
    prepped.rightCols(1) *= _alpha_scale;
    prepped.rightCols(4) = ClampValues(prepped.rightCols(4));

    for (int i = 0; i < num_triangles; i++) {
        const RowVector row = prepped.row(i);

        const BLRgba colour(row[6], row[7], row[8], row[9]);
        // clang-format off
        const BLTriangle triangle(
            x_scale * row[0], y_scale * row[1],
            x_scale * row[2], y_scale * row[3],
            x_scale * row[4], y_scale * row[5]
        );
        // clang-format on
        _context.fillTriangle(triangle, colour);
    }

    return errors::no_error;
}

Error Canvas::DrawFilledRectangles(ConstMatrixRef params) {
    const int num_rects = params.rows();
    const int num_dimensions = params.cols();
    if (num_dimensions != 8) {
        return Error(fmt::format("Expected a Nx8 array, got an {}x{}.", num_rects, num_dimensions));
    }

    // Scaling is isotropic, so vertical is [0,1] while horizontal is
    // [0, aspect].  This means getting to the full size image is just a matter
    // of multiplying by the height.

    const double x_scale = _context.targetWidth() - 1;
    const double y_scale = _context.targetHeight() - 1;

    // Force the shapes to be *mostly* inside of the frame but keep the colour
    // values clamped on [0, 1] since anything outside that doesn't make any
    // sense.  Apply the alpha scaling right before any clamping.

    Matrix prepped = params;
    prepped.leftCols(4) = 1.2 * RescaleValuesColumnWise(params.leftCols(4)).array() - 0.1;

    prepped.rightCols(4) = params.rightCols(4);
    prepped.rightCols(1) *= _alpha_scale;
    prepped.rightCols(4) = ClampValues(prepped.rightCols(4));

    for (int i = 0; i < num_rects; i++) {
        const RowVector row = prepped.row(i);

        const double x1 = x_scale * row[0];
        const double y1 = y_scale * row[1];
        const double x2 = x_scale * row[2];
        const double y2 = y_scale * row[3];

        const double x = std::min(x1, x2);
        const double y = std::min(y1, y2);
        const double w = std::abs(x1 - x2);
        const double h = std::abs(y1 - y2);

        const BLRgba colour(row[4], row[5], row[6], row[7]);
        const BLRect rect(x, y, w, h);
        _context.fillRect(rect, colour);
    }

    return errors::no_error;
}

void Canvas::RandomFill() {
    auto image = _context.targetImage();
    abstractions_assert(image != nullptr);

    BLImageData image_data;
    abstractions_assert(image->getData(&image_data) == BL_SUCCESS);

    uint8_t *buffer = static_cast<uint8_t *>(image_data.pixelData);
    size_t num_bytes = image_data.stride * image_data.size.h;
    std::generate(buffer, buffer + num_bytes, _prng);
}

void Canvas::SetAlphaScale(const double alpha_scale) {
    abstractions_assert(alpha_scale > 0 && alpha_scale <= 1.0);
    _alpha_scale = alpha_scale;
}

Error Canvas::SetCompositeMode(const CompositeMode mode) {
    BLCompOp op;
    switch (mode) {
        case CompositeMode::SrcCopy:
            op = BL_COMP_OP_SRC_COPY;
            break;
        case CompositeMode::SrcOver:
            op = BL_COMP_OP_SRC_OVER;
            break;
        default:
            return Error(fmt::format("Unknown compositing mode {}.", static_cast<int>(mode)));
    }

    _context.setCompOp(op);

    return errors::no_error;
}

}  // namespace abstractions::render
