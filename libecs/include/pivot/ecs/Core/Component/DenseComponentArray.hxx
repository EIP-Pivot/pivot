#pragma once

#include <span>

#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/ecs/Core/Component/error.hxx>
#include <pivot/ecs/Core/types.hxx>

namespace pivot::ecs::component
{

/** \brief A generic implementor of IComponentArray, optimized for components
 * used for nearly every entity.
 *
 * The DenseTypedComponentArray stores the values of a component in a contiguous
 * array. It is optimized for components which are present on most entities in a
 * Scene.
 */
template <typename T>
class DenseTypedComponentArray : public IComponentArray
{
public:
    /// Creates a DenseTypedComponentArray from the Description of its component
    DenseTypedComponentArray(Description description)
        : m_description(std::move(description)), m_component_exist(), m_components()
    {
    }

    /// \copydoc pivot::ecs::component::IComponentArray::getDescription()
    const Description &getDescription() const override { return m_description; }

    /// \copydoc pivot::ecs::component::IComponentArray::getValueForEntity()
    std::optional<data::Value> getValueForEntity(Entity entity) const override
    {
        if (entity >= m_components.size() || !m_component_exist[entity]) return std::nullopt;
        return helpers::Helpers<T>::createValueFromType(m_components.at(entity));
    }

    /// \copydoc pivot::ecs::component::IComponentArray::entityHasValue()
    bool entityHasValue(Entity entity) const override
    {
        return entity < m_component_exist.size() && m_component_exist[entity];
    }

    /// \copydoc pivot::ecs::component::IComponentArray::setValueForEntity()
    void setValueForEntity(Entity entity, std::optional<data::Value> value) override
    {
        if (!value.has_value()) {
            if (entity < m_components.size()) m_component_exist.at(entity) = false;
        } else {
            auto value_type = value->type();
            if (!value_type.isSubsetOf(m_description.type)) {
                throw InvalidComponentValue(m_description.name, m_description.type, value_type);
            }
            if (entity >= m_components.size()) {
                m_components.resize(entity + 1);
                m_component_exist.resize(entity + 1, false);
            }
            helpers::Helpers<T>::updateTypeWithValue(m_components.at(entity), value.value());
            m_component_exist.at(entity) = true;
        }
    }

    /// \copydoc pivot::ecs::component::IComponentArray::maxEntity()
    Entity maxEntity() const override { return m_components.size(); }

    /// Returns a mutable view into the components values. Some of those values can be nonsensical as the entity can
    /// miss this component.
    virtual std::span<T> getData() { return this->m_components; }

    /// Returns a constant view into the components values. Some of those values can be nonsensical as the entity can
    /// miss this component.
    virtual std::span<const T> getData() const { return this->m_components; }

    /// Returns the booleans specifying whether an an entity has the component
    virtual const std::vector<bool> &getExistence() const { return this->m_component_exist; }

    /// Returns the vector containing the component data
    virtual const std::vector<T> &getComponents() const { return this->m_components; }

protected:
    /// Description of the component
    Description m_description;

    /** \brief Vector of booleans indicating whether an entity has a component or not
     *
     * If m_component_exist[entityId] is true, then the entity has a value of the component in m_components[entityId].
     *
     * Otherwise, the entity has no value of the component, and the content of m_components[entityId] is undefined.
     */
    std::vector<bool> m_component_exist;

    /// The storage containing the values of the components for each entity.
    std::vector<T> m_components;
};
}    // namespace pivot::ecs::component
