#include <pivot/ecs/Components/TagArray.hxx>

namespace pivot::ecs::component
{
void TagArray::setValueForEntity(Entity entity, std::optional<data::Value> value)
{
    std::optional<std::string> old_name = (entity < m_component_exist.size() && m_component_exist[entity])
                                              ? std::make_optional(m_components[entity].name)
                                              : std::nullopt;

    this->DenseTypedComponentArray<Tag>::setValueForEntity(entity, value);

    // Remove old tag from tag set
    if (old_name.has_value()) { m_tag_names.erase(old_name.value()); }

    // Add new tag to tag set
    if (value.has_value()) {
        auto tag = std::get<std::string>(std::get<data::Record>(value.value()).at("name"));
        m_tag_names.insert(tag);
    }
}
}    // namespace pivot::ecs::component
