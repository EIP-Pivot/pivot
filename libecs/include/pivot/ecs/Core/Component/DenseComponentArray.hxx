#pragma once

#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/types.hxx>

namespace pivot::ecs::component
{
template <typename T>
class DenseTypedComponentArray : public IComponentArray
{
public:
    DenseTypedComponentArray(Description description): m_description(description) {}

    const Description &getDescription() const override { return m_description; }

    std::optional<std::any> getValueForEntity(Entity entity) const override
    {
        if (!m_component_exist.at(entity)) return std::nullopt;
        return std::make_any<T>(m_components.at(entity));
    }

    std::optional<std::any> getRefForEntity(Entity entity) override
    {
        if (!m_component_exist.at(entity)) return std::nullopt;
        return std::make_any<std::reference_wrapper<T>>(m_components.at(entity));
    }

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
    Description m_description;
    std::vector<bool> m_component_exist;
    std::vector<T> m_components;
};
}    // namespace pivot::ecs::component
