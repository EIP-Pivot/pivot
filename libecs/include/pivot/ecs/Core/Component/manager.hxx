#pragma once

#include "pivot/ecs/Core/Component/array.hxx"
#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/types.hxx"
#include <any>
#include <memory>

namespace pivot::ecs::component
{
class Manager
{
public:
    using ComponentId = ComponentType;

    ComponentId RegisterComponent(const Description &componentDescription)
    {
        if (m_componentNameToIndex.contains(componentDescription.name))
            throw EcsException("Registering component type more than once.");

        ComponentId index = m_componentArrays.size();
        m_componentArrays.push_back(componentDescription.createContainer());
        m_componentNameToIndex.insert({componentDescription.name, index});
        return index;
    }

    std::optional<ComponentId> GetComponentId(std::string_view name)
    {
        auto it = m_componentNameToIndex.find(name);
        if (it == m_componentNameToIndex.end()) {
            return std::nullopt;
        } else {
            return std::make_optional(it->second);
        }
    }

    // template <typename T>
    // ComponentType GetComponentType()
    // {
    //     const char *typeName = typeid(T).name();
    //     if (!mComponentTypes.contains(typeName))
    //         throw EcsException("Component not registered before use.");

    //     return mComponentTypes[typeName];
    // }

    void AddComponent(Entity entity, std::any component, ComponentId index)
    {
        m_componentArrays.at(index)->setValueForEntity(entity, component);
    }

    void RemoveComponent(Entity entity, ComponentId index)
    {
        m_componentArrays.at(index)->setValueForEntity(entity, std::nullopt);
    }

    const std::optional<std::any> GetComponent(Entity entity, ComponentId index) const
    {
        return m_componentArrays.at(index)->getValueForEntity(entity);
    }

    [[deprecated]] const std::optional<std::any> GetComponentRef(Entity entity, ComponentId index)
    {
        return m_componentArrays.at(index)->getRefForEntity(entity);
    }

    void EntityDestroyed(Entity entity)
    {
        for (auto &componentArray: m_componentArrays) { componentArray->setValueForEntity(entity, std::nullopt); }
    }

private:
    // std::unordered_map<std::string, ComponentType> mComponentTypes{};
    std::vector<std::unique_ptr<IComponentArray>> m_componentArrays;
    std::map<std::string, ComponentId, std::less<>> m_componentNameToIndex;
};
}    // namespace pivot::ecs::component
