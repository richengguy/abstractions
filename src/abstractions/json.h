#pragma once

#include <abstractions/math/types.h>
#include <abstractions/render/shapes.h>
#include <nlohmann/json.hpp>

namespace abstractions
{
void to_json(nlohmann::json &json, const Options<render::AbstractionShape> shapes);
void from_json(const nlohmann::json &json, Options<render::AbstractionShape> &shapes);

}

namespace nlohmann
{

template<>
struct adl_serializer<abstractions::ColumnVector>
{
    static void to_json(nlohmann::json &json, const abstractions::ColumnVector &vector);
    static void from_json(const nlohmann::json &json, abstractions::ColumnVector &vector);
};

template<>
struct adl_serializer<abstractions::RowVector>
{
    static void to_json(nlohmann::json &json, const abstractions::RowVector &vector);
    static void from_json(const nlohmann::json &json, abstractions::RowVector &vector);
};

template<>
struct adl_serializer<abstractions::Matrix>
{
    static void to_json(nlohmann::json &json, const abstractions::Matrix &matrix);
    static void from_json(const nlohmann::json &json, abstractions::Matrix &matrix);
};

}
