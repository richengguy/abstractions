#include "json.h"

namespace abstractions
{

void to_json(nlohmann::json &json, const Options<render::AbstractionShape> shapes)
{
    json = nlohmann::json::array();

    if (shapes & render::AbstractionShape::Circles) {
        json.push_back("circles");
    }

    if (shapes & render::AbstractionShape::Rectangles) {
        json.push_back("rectangles");
    }

    if (shapes & render::AbstractionShape::Triangles) {
        json.push_back("triangles");
    }
}

void from_json(const nlohmann::json &json, Options<render::AbstractionShape> &shapes)
{
    for (int i = 0; i < json.size(); i++)
    {
        auto str = json.at(i).get<std::string>();
        if (str == "circles") {
            shapes.Set(render::AbstractionShape::Circles);
        } else if (str == "rectangles") {
            shapes.Set(render::AbstractionShape::Rectangles);
        } else if (str == "triangles") {
            shapes.Set(render::AbstractionShape::Triangles);
        }
    }
}

}
