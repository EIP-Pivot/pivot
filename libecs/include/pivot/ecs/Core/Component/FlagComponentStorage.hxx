#pragma once

#include <set>

#include <pivot/ecs/Core/Component/array.hxx>

namespace pivot::ecs::component
{
/// Storage class for components without content, which only serve to "flag" entities
class FlagComponentStorage : public IComponentArray
{
public:
    /// Creates a FlagComponentStorage from the Description of its component
    FlagComponentStorage(Description description): m_description(std::move(description)) {}

    /// \copydoc pivot::ecs::component::IComponentArray::getDescription()
    const Description &getDescription() const override;

    /// \copydoc pivot::ecs::component::IComponentArray::getValueForEntity()
    std::optional<data::Value> getValueForEntity(Entity entity) const override;

    /// \copydoc pivot::ecs::component::IComponentArray::entityHasValue()
    bool entityHasValue(Entity entity) const override;

    /// \copydoc pivot::ecs::component::IComponentArray::setValueForEntity()
    void setValueForEntity(Entity entity, std::optional<data::Value> value) override;

    /// \copydoc pivot::ecs::component::IComponentArray::maxEntity()
    Entity maxEntity() const override;

    /// Get acces to the set storing whether an entity has the component
    const std::set<Entity> &getData();

private:
    /// Description of the component
    Description m_description;
    /// Storage of the component existence or not
    std::set<Entity> m_entity_having_component;
    /// The maximum entity id that ever had a value in this storage
    Entity m_max_entity = 0;
};

}    // namespace pivot::ecs::component
