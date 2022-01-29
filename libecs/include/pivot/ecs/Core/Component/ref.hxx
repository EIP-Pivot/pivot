#pragma once

#include <pivot/ecs/Core/Component/array.hxx>

namespace pivot::ecs::component
{
class ComponentRef
{
public:
    ComponentRef(IComponentArray &array, Entity entity): m_array(array), m_entity(entity) {}

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
        MissingComponent(const Description &component, Entity entity);

        const Description &component;
        Entity entity;
    };

private:
    IComponentArray &m_array;
    Entity m_entity;
};
}    // namespace pivot::ecs::component
