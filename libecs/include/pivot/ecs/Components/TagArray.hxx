#pragma once

#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/UniqueComponentArray.hxx>

namespace pivot::ecs::component
{

/** \brief Storage for Tag components
 *
 * This IComponentArray is a DenseTypedComponentArray, but it additionally
 * checks that every entity has always a duplicate Tag, by throwing a
 * DuplicateEntityTag exception when a duplicated Tag is registered.
 */
class TagArray : public UniqueComponentArray<Tag>
{
public:
    /// Creates a TagArray
    TagArray(Description d): UniqueComponentArray<Tag>(d) {}

    /// Get the id of an Entity by its name
    std::optional<Entity> getEntityID(const std::string &name)
    {
        auto it = m_unique_hash.find(std::hash<Tag>()(Tag{name}));
        if (it == m_unique_hash.end()) {
            return std::nullopt;
        } else {
            return std::make_optional(it->second);
        }
    }
};

}    // namespace pivot::ecs::component
