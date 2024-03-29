#include "pivot/ecs/Core/Component/manager.hxx"

namespace pivot::ecs::component
{
Manager::ComponentId Manager::RegisterComponent(const Description &componentDescription)
{
    if (m_componentNameToIndex.contains(componentDescription.name))
        throw EcsException("Registering component type more than once.");

    ComponentId index = m_componentArrays.size();
    m_componentArrays.push_back(componentDescription.createContainer(componentDescription));
    m_componentNameToIndex.insert({componentDescription.name, index});
    return index;
}

std::optional<Manager::ComponentId> Manager::GetComponentId(std::string_view name) const
{
    auto it = m_componentNameToIndex.find(name);
    if (it == m_componentNameToIndex.end()) {
        return std::nullopt;
    } else {
        return std::make_optional(it->second);
    }
}

void Manager::AddComponent(Entity entity, data::Value component, Manager::ComponentId index)
{
    m_componentArrays.at(index)->setValueForEntity(entity, component);
}

void Manager::RemoveComponent(Entity entity, Manager::ComponentId index)
{
    m_componentArrays.at(index)->setValueForEntity(entity, std::nullopt);
}

/// Get the value of a component associated to an entity
const std::optional<data::Value> Manager::GetComponent(Entity entity, Manager::ComponentId index) const
{
    return m_componentArrays.at(index)->getValueForEntity(entity);
}

/// Removes the component for every entity.
void Manager::EntityDestroyed(Entity entity)
{
    for (auto &componentArray: m_componentArrays) { componentArray->setValueForEntity(entity, std::nullopt); }
}

IComponentArray &Manager::GetComponentArray(ComponentId index) { return *m_componentArrays.at(index); }

}    // namespace pivot::ecs::component
