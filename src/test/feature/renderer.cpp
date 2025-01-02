#include "support.h"

#include <abstractions/render/shapes.h>
#include <abstractions/render/renderer.h>

using namespace abstractions;
using namespace abstractions::render;

constexpr int kNumShapes = 10;
constexpr int kWidth = 1024;
constexpr int kHeight = 728;

ABSTRACTIONS_FEATURE_TEST()
{
    ShapeGenerator generator(kWidth, kHeight, prng);
    auto random_circles = generator.RandomCircles(kNumShapes);
    auto random_rectangles = generator.RandomRectangles(kNumShapes);
    auto random_triangles = generator.RandomTriangles(kNumShapes);

    console.Print("Init renderer.");
    auto renderer = Renderer::Create(kWidth, kHeight, prng);
    abstractions_check(renderer);

    console.Print("Rendering {} random shapes.", kNumShapes);
    {
        PackedShapeCollection shapes(random_circles, random_rectangles, random_triangles);
        renderer->Render(shapes);
        renderer->DrawingSurface().Save(output_folder.FilePath("all-shapes.png"));
    }

    console.Print("Rendering triangles with random background.", kNumShapes);
    {
        PackedShapeCollection shapes(CircleCollection(0), RectangleCollection(0), random_triangles);
        renderer->UseRandomBackgroundFill(true);
        renderer->Render(shapes);
        renderer->DrawingSurface().Save(output_folder.FilePath("triangles-random-bg.png"));
    }

    console.Print("Render a dark-green background without any shapes.");
    {
        PackedShapeCollection shapes(CircleCollection(0), RectangleCollection(0), TriangleCollection(0));
        renderer->UseRandomBackgroundFill(false);
        renderer->SetBackground(0, 128, 0);
        renderer->Render(shapes);
        renderer->DrawingSurface().Save(output_folder.FilePath("bg-only.png"));
    }
}

ABSTRACTIONS_FEATURE_TEST_MAIN("renderer", "Generate some simple images using the abstractions renderer.");
