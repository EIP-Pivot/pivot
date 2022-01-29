#pragma once

#include <nlohmann/json.hpp>

#include <pivot/ecs/Core/Data/value.hxx>

namespace pivot::ecs::data
{
/// Serialize a Value to json
void to_json(nlohmann::json &json, const Value &value);

/// Deserialize a Value from json
void from_json(const nlohmann::json &json, Value &value);
}    // namespace pivot::ecs::data

namespace nlohmann
{
template <>
/// Specialization to allow serializing and deserializing glm::vec3
struct adl_serializer<glm::vec3> {
    /// Specialization to allow serializing and deserializing glm::vec3
    static void to_json(json &j, const glm::vec3 &opt);
    /// Specialization to allow serializing and deserializing glm::vec3
    static void from_json(const json &j, glm::vec3 &opt);
};
}    // namespace nlohmann
