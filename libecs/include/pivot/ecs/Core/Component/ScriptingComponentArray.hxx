#pragma once

#include "pivot/ecs/Core/Component/array.hxx"

namespace pivot::ecs::component
{

class ScriptingComponentArray : public IComponentArray
{
public:
    /// Creates a ScriptingComponentArray from the Description of its component
    ScriptingComponentArray(Description description): m_description(std::move(description)) {}

    /// \copydoc pivot::ecs::component::IComponentArray::getDescription()
    const Description &getDescription() const override;

    /// \copydoc pivot::ecs::component::IComponentArray::getValueForEntity()
    std::optional<data::Value> getValueForEntity(Entity entity) const override;

    void setValueForEntity(Entity entity, std::optional<data::Value> value) override;

    /// \copydoc pivot::ecs::component::IComponentArray::entityHasValue()
    bool entityHasValue(Entity entity) const override
    {
        return m_components.contains(entity); // FPOINT : entry in map without value ?
    }

    /// \copydoc pivot::ecs::component::IComponentArray::maxEntity()
    Entity maxEntity() const override { return m_components.size() - 1; }

protected:
    /// Description of the component
    Description m_description;

    std::unordered_map<Entity, data::Value> m_components;
};

}    // namespace pivot::ecs::component