#pragma once

#include <nlohmann/json.hpp>

#include <pivot/ecs/Core/Data/value.hxx>

namespace pivot::ecs::data
{
void to_json(nlohmann::json &json, const Value &value);
void from_json(const nlohmann::json &json, Value &value);
}    // namespace pivot::ecs::data

namespace nlohmann
{
template <>
struct adl_serializer<glm::vec3> {
    static void to_json(json &j, const glm::vec3 &opt);
    static void from_json(const json &j, glm::vec3 &opt);
};
}    // namespace nlohmann
