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
    ComponentId RegisterComponent(const Description &componentDescription);

    /// Get the id of a component if it was registered
    std::optional<ComponentId> GetComponentId(std::string_view name);

    /// Add or replace the component associated to an entity
    void AddComponent(Entity entity, std::any component, ComponentId index);

    /// Remove the component associated to an entity
    void RemoveComponent(Entity entity, ComponentId index);

    /// Get the value of a component associated to an entity
    const std::optional<std::any> GetComponent(Entity entity, ComponentId index) const;

    /// Get a reference to the value of a component associated to an entity
    [[deprecated]] const std::optional<std::any> GetComponentRef(Entity entity, ComponentId index);

    /// Removes the component for every entity.
    void EntityDestroyed(Entity entity);

private:
    std::vector<std::unique_ptr<IComponentArray>> m_componentArrays;
    std::map<std::string, ComponentId, std::less<>> m_componentNameToIndex;
};
}    // namespace pivot::ecs::component
