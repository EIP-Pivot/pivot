#include <nlohmann/json.hpp>
#include "pivot/ecs/Core/Persistence/Component.hxx"

namespace pivot::ecs::persistence
{
    std::string serializeComponent(const pivot::ecs::component::Description &description, const std::any &component)
    {
        nlohmann::json serialized;
        serialized["name"] = description.name;
        for (auto property: description.properties) {
            auto value = description.getProperty(component, property.name);
            std::visit([&](auto &&value){
                using T = std::decay_t<decltype(value)>;
                serialized["properties"][property.name] = value;
            }, value);
        }
        return serialized.dump();
    }
}