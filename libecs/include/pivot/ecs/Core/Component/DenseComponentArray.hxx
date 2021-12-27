#pragma once

#include <pivot/ecs/Core/Component/array.hxx>
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
        : m_description(description), m_component_exist(MAX_ENTITIES, false), m_components(MAX_ENTITIES)
    {
    }

    /// \copydoc pivot::ecs::component::IComponentArray::getDescription()
    const Description &getDescription() const override { return m_description; }

    /// \copydoc pivot::ecs::component::IComponentArray::getValueForEntity()
    std::optional<std::any> getValueForEntity(Entity entity) const override
    {
        if (!m_component_exist.at(entity)) return std::nullopt;
        return std::make_any<T>(m_components.at(entity));
    }

    /// \copydoc pivot::ecs::component::IComponentArray::getRefForEntity()
    std::optional<std::any> getRefForEntity(Entity entity) override
    {
        if (!m_component_exist.at(entity)) return std::nullopt;
        return std::make_any<std::reference_wrapper<T>>(m_components.at(entity));
    }

    /// \copydoc pivot::ecs::component::IComponentArray::setValueForEntity()
    void setValueForEntity(Entity entity, std::optional<std::any> value) override
    {
        if (!value.has_value()) {
            m_component_exist.at(entity) = false;
        } else {
            m_components.at(entity) = std::any_cast<T>(value.value());
            m_component_exist.at(entity) = true;
        }
    }

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
