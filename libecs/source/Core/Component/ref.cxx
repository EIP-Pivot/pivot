#include <pivot/ecs/Core/Component/ref.hxx>

namespace pivot::ecs::component
{
data::Value ComponentRef::get() const
{
    auto value = m_array.getValueForEntity(m_entity);
    if (!value) { throw MissingComponent(m_array.getDescription(), m_entity); }
    return *value;
}

void ComponentRef::set(const data::Value &value) { m_array.setValueForEntity(m_entity, value); }

ComponentRef::operator data::Value() const { return this->get(); }

void ComponentRef::operator=(const data::Value &value) { this->set(value); }

ComponentRef::MissingComponent::MissingComponent(const Description &component, Entity entity)
    : std::logic_error(std::string("Missing component " + component.name + " on entity " + std::to_string(entity))),
      component(component),
      entity(entity)
{
}

}    // namespace pivot::ecs::component
