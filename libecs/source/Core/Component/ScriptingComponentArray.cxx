#include "pivot/ecs/Core/Component/ScriptingComponentArray.hxx"

namespace pivot::ecs::component
{

const Description &ScriptingComponentArray::getDescription() const { return m_description; }

std::optional<data::Value> ScriptingComponentArray::getValueForEntity(Entity entity) const
{
    auto value = m_components.find(entity);
    if (value == m_components.end()) return std::nullopt;
    return value->second;
}

void ScriptingComponentArray::setValueForEntity(Entity entity, std::optional<data::Value> value)
{
    if (!value) {
        m_components.erase(entity);
    } else {
        m_components[entity] = value.value();
        m_max_entity = std::max(entity, m_max_entity);
    }
}

bool ScriptingComponentArray::entityHasValue(Entity entity) const { return m_components.contains(entity); }

Entity ScriptingComponentArray::maxEntity() const { return m_max_entity; }

}    // namespace pivot::ecs::component
