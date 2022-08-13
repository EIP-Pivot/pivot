#include <pivot/ecs/Components/TagArray.hxx>

namespace pivot::ecs::component
{
void TagArray::setValueForEntity(Entity entity, std::optional<data::Value> value)
{
    std::optional<std::string> old_name = (entity < m_component_exist.size() && m_component_exist[entity])
                                              ? std::make_optional(m_components[entity].name)
                                              : std::nullopt;

    if (value.has_value()) {
        Tag tag = this->parseValue(value.value());

        // Check that the new name is not already used
        if (m_tag_names.contains(tag.name)) { throw DuplicateEntityTag(tag.name); }

        // Add new tag to tag set
        m_tag_names.insert(tag.name);
    }

    // Remove old tag from tag set
    if (old_name.has_value()) { m_tag_names.erase(old_name.value()); }

    // Update tag component
    this->DenseTypedComponentArray<Tag>::setValueForEntity(entity, value);
}
}    // namespace pivot::ecs::component
