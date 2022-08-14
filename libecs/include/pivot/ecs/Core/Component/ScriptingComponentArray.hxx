#pragma once

#include "pivot/ecs/Core/Component/array.hxx"

namespace pivot::ecs::component
{

/** \brief A generic implementor of IComponentArray, optimized for script components.
 *
 * The ScriptingComponentArray stores the values of a script component in a contiguous
 * array. It is optimized for components which are present on most entities in a
 * Scene.
 */
class ScriptingComponentArray : public IComponentArray
{
public:
    /// Creates a ScriptingComponentArray from the Description of its component
    ScriptingComponentArray(Description description): m_description(std::move(description)) {}

    /// \copydoc pivot::ecs::component::IComponentArray::getDescription()
    const Description &getDescription() const override;

    std::optional<data::Value> getValueForEntity(Entity entity) const override;

    void setValueForEntity(Entity entity, std::optional<data::Value> value) override;

    bool entityHasValue(Entity entity) const override;

    Entity maxEntity() const override;

protected:
    /// Description of the component
    Description m_description;
    /// The storage containing the values of the components for each entity.
    std::unordered_map<Entity, data::Value> m_components;
    /// The maximum entity id that ever had a value in this array
    Entity m_max_entity = 0;
};

}    // namespace pivot::ecs::component
