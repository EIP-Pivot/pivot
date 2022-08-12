#include <pivot/ecs/Core/Data/value_serialization.hxx>

namespace pivot::ecs::data
{
void to_json(nlohmann::json &json, const Value &value)
{
    std::visit([&](auto &data) { json = data; }, static_cast<const Value::variant &>(value));
}

void from_json(const nlohmann::json &json, Value &value)
{
    if (json.is_string()) {
        value = json.get<std::string>();
    } else if (json.is_number_float()) {
        value = json.get<double>();
    } else if (json.is_number_integer()) {
        value = json.get<int>();
    } else if (json.is_boolean()) {
        value = json.get<bool>();
    } else if (json.is_array()) {
        if (json.size() != 3) { throw nlohmann::json::type_error::create(399, "Invalid data::Value", json); }
        value = json.get<glm::vec3>();
    } else if (json.is_object()) {
        // TODO: How to distinguish between an asset and a record ?
        if (json.contains("asset") && json.size() == 1)
            value = json.get<Asset>();
        else if (json.contains("entity") && json.size() == 1)
            value = json.get<EntityRef>();
        else
            value = json.get<Record>();
    } else if (json.is_null()) {
        value = Void{};
    } else {
        throw nlohmann::json::type_error::create(399, "Invalid data::Value", json);
    }
}

std::ostream &operator<<(std::ostream &stream, const Record &type) { return stream << nlohmann::json(type).dump(); }
std::ostream &operator<<(std::ostream &stream, const Value &type) { return stream << nlohmann::json(type).dump(); }

/// Serialize an Asset to json
void to_json(nlohmann::json &json, const Asset &value) { json["asset"]["name"] = value.name; }

/// Deserialize an Asset from json
void from_json(const nlohmann::json &json, Asset &value) { value.name = json["asset"]["name"].get<std::string>(); }

/// Serialize Void to json
void to_json([[maybe_unused]] nlohmann::json &json, [[maybe_unused]] const Void &value) {}

/// Deserialize Void from json
void from_json([[maybe_unused]] const nlohmann::json &json, [[maybe_unused]] Void &value) {}

/// Serialize an EntityRef to json
void to_json(nlohmann::json &json, const EntityRef &value)
{
    if (value.is_empty()) {
        json["entity"] = nullptr;
    } else {
        json["entity"] = value.ref;
    }
}

/// Deserialize an EntityRef from json
void from_json(const nlohmann::json &json, EntityRef &value)
{
    if (json["entity"].is_null()) {
        value = EntityRef::empty();
    } else {
        value.ref = json["entity"].get<Entity>();
    }
}
}    // namespace pivot::ecs::data

namespace nlohmann
{
void adl_serializer<glm::vec3>::to_json(json &j, const glm::vec3 &opt) { j = {opt.x, opt.y, opt.z}; }

void adl_serializer<glm::vec3>::from_json(const json &j, glm::vec3 &opt)
{
    opt.x = j.at(0).get<float>();
    opt.y = j.at(1).get<float>();
    opt.z = j.at(2).get<float>();
}

}    // namespace nlohmann
