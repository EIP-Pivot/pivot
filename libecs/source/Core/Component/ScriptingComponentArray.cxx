#include "pivot/ecs/Core/Component/ScriptingComponentArray.hxx"

namespace pivot::ecs::component
{

const Description &ScriptingComponentArray::getDescription() const
{ 
    return m_description;
}

std::optional<data::Value> ScriptingComponentArray::getValueForEntity(Entity entity) const
{
    auto value = m_components.find(entity);
    if (value == m_components.end())
        return std::nullopt;
    return value->second;
}

void ScriptingComponentArray::setValueForEntity(Entity entity, std::optional<data::Value> value)
{
    m_components[entity] = value.value();
}


} // end namespace pivot::ecs::component