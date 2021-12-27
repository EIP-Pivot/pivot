#pragma once

#include <any>
#include <memory>
#include <ranges>

#include "pivot/ecs/Core/Component/array.hxx"
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
    using component_array_type = std::vector<std::unique_ptr<IComponentArray>>;
    using value_type = std::pair<const Description &, std::optional<std::any>>;

    component_array_type m_componentArrays;
    std::map<std::string, ComponentId, std::less<>> m_componentNameToIndex;

public:
    /// Returns a range containing all components of the entity
    auto GetAllComponents(Entity entity)
    {
        auto components =
            std::views::transform(m_componentArrays, [entity](std::unique_ptr<IComponentArray> &component_array) {
                return std::make_pair(component_array->getDescription(), component_array->getValueForEntity(entity));
            });
        auto filtered = std::views::filter(components, [](auto pair) { return pair.second.has_value(); });
        return std::views::transform(filtered,
                                     [](auto pair) { return std::make_pair(pair.first, pair.second.value()); });
    }
};
}    // namespace pivot::ecs::component
