#include "abstractions/render/canvas.h"

#include <abstractions/errors.h>
#include <fmt/format.h>

#include <algorithm>

namespace abstractions {

Canvas::Canvas(Image &image, std::optional<DefaultRngType::result_type> seed) :
    _context{image},
    _prng{seed.value_or(PrngGenerator<DefaultRngType>::DrawRandomSeed())} {}

Canvas::Canvas(Image &image, Prng<DefaultRngType> &prng) :
    _context{image},
    _prng{prng} {}

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

    const double scale = _context.targetHeight() - 1;

    for (int i = 0; i < num_circles; i++) {
        const RowVector row = params.row(i);

        const BLRgba colour(row[3], row[4], row[5], row[6]);
        // clang-format off
        const BLCircle circle(
            scale * row[0],
            scale * row[1],
            scale * std::abs(row[2])
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

    const double scale = _context.targetHeight() - 1;

    for (int i = 0; i < num_triangles; i++) {
        const RowVector row = params.row(i);

        const BLRgba colour(row[6], row[7], row[8], row[9]);
        // clang-format off
        const BLTriangle triangle(
            scale * row[0], scale * row[1],
            scale * row[2], scale * row[3],
            scale * row[4], scale * row[5]
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

    const double scale = _context.targetHeight() - 1;

    for (int i = 0; i < num_rects; i++) {
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

    return errors::no_error;
}

void Canvas::RandomFill() {
    auto image = _context.targetImage();
    abstractions_assert(image != nullptr);

    BLImageData image_data;
    auto err = image->getData(&image_data);
    abstractions_assert(err == BL_SUCCESS);

    uint8_t *buffer = static_cast<uint8_t *>(image_data.pixelData);
    size_t num_bytes = image_data.stride * image_data.size.h;
    std::generate(buffer, buffer + num_bytes, _prng);
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

}  // namespace abstractions
