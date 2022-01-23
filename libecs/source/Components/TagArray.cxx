#include <pivot/ecs/Components/TagArray.hxx>

namespace pivot::ecs::component
{
void TagArray::setValueForEntity(Entity entity, std::optional<std::any> value)
{
    // Remove old tag from tag set
    if (entity < m_component_exist.size() && m_component_exist[entity]) {
        m_tag_names.erase(m_components[entity].name);
    }

    // Add new tag to tag set
    if (value.has_value()) {
        auto tag = std::any_cast<Tag>(*value);
        m_tag_names.insert(tag.name);
    }

    this->DenseTypedComponentArray<Tag>::setValueForEntity(entity, value);
}
}    // namespace pivot::ecs::component
