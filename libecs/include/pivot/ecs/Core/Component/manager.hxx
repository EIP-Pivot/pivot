#pragma once

// Important ! Do not change the order
#include <queue>
#include <ranges>
// because msvc suck! Thx

#include <any>
#include <memory>

#include "pivot/ecs/Core/Component/array.hxx"
#include "pivot/ecs/Core/Component/ref.hxx"
#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/types.hxx"

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
    void AddComponent(Entity entity, data::Value component, ComponentId index);

    /// Remove the component associated to an entity
    void RemoveComponent(Entity entity, ComponentId index);

    /// Get the value of a component associated to an entity
    const std::optional<data::Value> GetComponent(Entity entity, ComponentId index) const;

    /// Removes the component for every entity.
    void EntityDestroyed(Entity entity);

    /// Get the component array of ComponentId.
    std::optional<std::reference_wrapper<IComponentArray>> GetComponentArray(ComponentId index);

private:
    using component_array_type = std::vector<std::unique_ptr<IComponentArray>>;
    using value_type = std::pair<const Description &, std::optional<data::Value>>;

    component_array_type m_componentArrays;
    std::map<std::string, ComponentId, std::less<>> m_componentNameToIndex;

public:
    /// Returns a range containing all components of the entity
    auto GetAllComponents(Entity entity)
    {
        return m_componentArrays |
               std::views::filter([entity](const auto &array) { return array->entityHasValue(entity); }) |
               std::views::transform([entity](auto &array) { return ComponentRef(*array, entity); });
    }
};
}    // namespace pivot::ecs::component
