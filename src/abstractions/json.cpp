#include "json.h"

#include <vector>

#include <abstractions/errors.h>

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

namespace nlohmann
{

namespace
{

template<typename T>
void matrix_to_json(nlohmann::json &json, const Eigen::MatrixBase<T> &matrix)
{
    json["dim"] = {
        {"r", matrix.rows()},
        {"c", matrix.cols()},
    };
    json["val"] = matrix.reshaped();
}

template<typename T>
void matrix_from_json(const nlohmann::json &json, Eigen::MatrixBase<T> &matrix)
{
    int num_rows = json["dim"]["r"].get<int>();
    int num_cols = json["dim"]["c"].get<int>();
    int length_data = json["val"].size();
    abstractions_assert(num_rows * num_cols == length_data);

    matrix = Eigen::MatrixBase<T>::Zero(num_rows, num_cols);
    for (int i = 0; i < length_data; i++)
    {
        matrix(i) = json["val"][i];
    }
}

}

void adl_serializer<abstractions::ColumnVector>::to_json(nlohmann::json &json, const abstractions::ColumnVector &vector)
{
    matrix_to_json(json, vector);
}

void adl_serializer<abstractions::ColumnVector>::from_json(const nlohmann::json &json, abstractions::ColumnVector &vector)
{
    matrix_from_json(json, vector);
}

void adl_serializer<abstractions::RowVector>::to_json(nlohmann::json &json, const abstractions::RowVector &vector)
{
    matrix_to_json(json, vector);
}

void adl_serializer<abstractions::RowVector>::from_json(const nlohmann::json &json, abstractions::RowVector &vector)
{
    matrix_from_json(json, vector);
}

void adl_serializer<abstractions::Matrix>::to_json(nlohmann::json &json, const abstractions::Matrix &matrix)
{
    matrix_to_json(json, matrix);
}

void adl_serializer<abstractions::Matrix>::from_json(const nlohmann::json &json, abstractions::Matrix &matrix)
{
    matrix_from_json(json, matrix);
}

}
