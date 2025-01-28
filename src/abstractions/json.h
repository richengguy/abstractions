#pragma once

#include <abstractions/render/shapes.h>
#include <nlohmann/json.hpp>

namespace abstractions
{
void to_json(nlohmann::json &json, const Options<render::AbstractionShape> shapes);
void from_json(const nlohmann::json &json, Options<render::AbstractionShape> &shapes);
}
