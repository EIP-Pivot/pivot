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

    std::optional<data::Value> getValueForEntity(Entity entity) const override;

    void setValueForEntity(Entity entity, std::optional<data::Value> value) override;

protected:
    /// Description of the component
    Description m_description;

    std::unordered_map<Entity, data::Value> m_components;
};

}    // namespace pivot::ecs::component