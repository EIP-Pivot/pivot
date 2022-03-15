#pragma once

#include <pivot/ecs/Core/Component/array.hxx>

namespace pivot::ecs::component
{

/** \brief A reference to a component of an entity
 *
 * This class wraps an entity and a component array to allow retrieving and
 * modifying the value of the component as if it was a normal reference.
 */
class ComponentRef
{
public:
    /// Basic ComponentRef constructor
    ComponentRef(IComponentArray &array, Entity entity): m_array(array), m_entity(entity) {}

    /// Get the description of the component
    const Description &description() const;

    /// Throws MissingComponent error if the component has no value
    data::Value get() const;

    /// Sets the value of the component in its underlying container
    void set(const data::Value &value);

    /// Same as get()
    operator data::Value() const;

    /// Same as set()
    void operator=(const data::Value &value);

    /// Error thrown when a component ref not refering to an existing compoent is dereference
    struct MissingComponent : std::logic_error {
        /// Creates a MissingComponent error
        MissingComponent(const Description &component, Entity entity);

        /// The component that was not found
        const Description &component;

        /// The entity on which the component was not found
        Entity entity;
    };

private:
    IComponentArray &m_array;
    Entity m_entity;
};
}    // namespace pivot::ecs::component
