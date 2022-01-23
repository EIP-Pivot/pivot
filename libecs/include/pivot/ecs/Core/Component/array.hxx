#pragma once
#include <array>
#include <optional>
#include <unordered_map>

#include <pivot/ecs/Core/Component/description.hxx>
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
    virtual std::optional<std::any> getValueForEntity(Entity entity) const = 0;

    // HACK: Remove this as soon as it is not needed anymore
    /// Does the same as getValueForEntity(), but returns a std::reference_wrapper of the type
    virtual std::optional<std::any> getRefForEntity(Entity entity) = 0;

    /// Sets the value of the component for an entity
    virtual void setValueForEntity(Entity entity, std::optional<std::any>) = 0;
};
}    // namespace pivot::ecs::component
