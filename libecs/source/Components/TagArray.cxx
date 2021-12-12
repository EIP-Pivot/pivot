#include <pivot/ecs/Components/TagArray.hxx>

namespace pivot::ecs::component
{
void TagArray::setValueForEntity(Entity entity, std::optional<std::any> value)
{
    if (!value.has_value()) {
        if (m_component_exist.at(entity)) { m_tag_names.erase(m_components.at(entity).name); }
        m_component_exist.at(entity) = false;
    } else {
        auto tag = std::any_cast<Tag>(value.value());

        if (m_tag_names.contains(tag.name)) { throw TagArray::DuplicateEntityTag(tag.name); }
        m_tag_names.insert(tag.name);

        m_components.at(entity) = tag;
        m_component_exist.at(entity) = true;
    }
}
}    // namespace pivot::ecs::component
