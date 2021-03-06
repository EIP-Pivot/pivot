#pragma once
#include <array>
#include <optional>
#include <unordered_map>

#include <pivot/ecs/Core/Component/description.hxx>
#include <pivot/ecs/Core/Data/value.hxx>
#include <pivot/ecs/Core/types.hxx>

namespace pivot::ecs::component
{
/** \brief Stores all the components of one type in a Scene
 *
 * This interface describes a storage to associate the value of a component
 * to an entity. It only manages a single type of component. For example the
 * TagArray can only contain Tag components.
 *
 * A component array must support setting, removing, and getting the
 * component associated to any entity.
 */
class IComponentArray
{
public:
    virtual ~IComponentArray() = default;

    /// Returns the Description of the component type managed by this component array.
    virtual const Description &getDescription() const = 0;

    /// Returns the value of the component associated with a given entity, if any.
    virtual std::optional<data::Value> getValueForEntity(Entity entity) const = 0;

    /// Returns true if the entity has a component value in his array
    virtual bool entityHasValue(Entity entity) const = 0;

    /// Sets the value of the component for an entity
    virtual void setValueForEntity(Entity entity, std::optional<data::Value>) = 0;

    /// Returns the largest entity which can have a value in the array
    virtual Entity maxEntity() const = 0;
};
}    // namespace pivot::ecs::component
