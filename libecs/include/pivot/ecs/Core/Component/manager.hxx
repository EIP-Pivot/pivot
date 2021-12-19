#pragma once

#include "pivot/ecs/Core/Component/array.hxx"
#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/types.hxx"
#include <any>
#include <memory>

namespace pivot::ecs::component
{

/** \brief Manages all the components in a Scene
 *
 * The Manager stores all the IComponentArray of all the components registered
 * in the scene. It assigns a ComponentId to each component type registered in
 * the Scene.
 */
class Manager
{
public:
    /// Numerical id associated to a component type when it is first registered in the Scene.
    using ComponentId = ComponentType;

    /** \brief Registers a component in the scene.
     *
     * Its corresponsing IComponentArray is created.
     */
    ComponentId RegisterComponent(const Description &componentDescription)
    {
        if (m_componentNameToIndex.contains(componentDescription.name))
            throw EcsException("Registering component type more than once.");

        ComponentId index = m_componentArrays.size();
        m_componentArrays.push_back(componentDescription.createContainer(componentDescription));
        m_componentNameToIndex.insert({componentDescription.name, index});
        return index;
    }

    /// Get the id of a component if it was registered
    std::optional<ComponentId> GetComponentId(std::string_view name)
    {
        auto it = m_componentNameToIndex.find(name);
        if (it == m_componentNameToIndex.end()) {
            return std::nullopt;
        } else {
            return std::make_optional(it->second);
        }
    }

    /// Add or replace the component associated to an entity
    void AddComponent(Entity entity, std::any component, ComponentId index)
    {
        m_componentArrays.at(index)->setValueForEntity(entity, component);
    }

    /// Remove the component associated to an entity
    void RemoveComponent(Entity entity, ComponentId index)
    {
        m_componentArrays.at(index)->setValueForEntity(entity, std::nullopt);
    }

    /// Get the value of a component associated to an entity
    const std::optional<std::any> GetComponent(Entity entity, ComponentId index) const
    {
        return m_componentArrays.at(index)->getValueForEntity(entity);
    }

    /// Get a reference to the value of a component associated to an entity
    [[deprecated]] const std::optional<std::any> GetComponentRef(Entity entity, ComponentId index)
    {
        return m_componentArrays.at(index)->getRefForEntity(entity);
    }

    /// Removes the component for every entity.
    void EntityDestroyed(Entity entity)
    {
        for (auto &componentArray: m_componentArrays) { componentArray->setValueForEntity(entity, std::nullopt); }
    }

private:
    std::vector<std::unique_ptr<IComponentArray>> m_componentArrays;
    std::map<std::string, ComponentId, std::less<>> m_componentNameToIndex;
};
}    // namespace pivot::ecs::component
