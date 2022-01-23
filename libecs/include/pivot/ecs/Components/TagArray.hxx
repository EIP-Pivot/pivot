#pragma once

#include <set>
#include <stdexcept>

#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/array.hxx>

namespace pivot::ecs::component
{

/** \brief Storage for Tag components
 *
 * This IComponentArray is a DenseTypedComponentArray, but it additionally
 * checks that every entity has always a duplicate Tag, by throwing a
 * DuplicateEntityTag exception when a duplicated Tag is registered.
 */
class TagArray : public DenseTypedComponentArray<Tag>
{
public:
    /// Creates a TagArray using the Description of Tag.
    TagArray(Description d): DenseTypedComponentArray<Tag>(d) {}

    void setValueForEntity(Entity entity, std::optional<std::any> value) override;

    /// Error thrown when a duplicate Tag is registered
    class DuplicateEntityTag : public std::logic_error
    {
    public:
        /// Creates a DuplicateEntityTag with the duplicated tag
        DuplicateEntityTag(const std::string &tag): std::logic_error("Duplicate entity tag: " + tag) {}
    };

private:
    std::set<std::string> m_tag_names;
};

}    // namespace pivot::ecs::component
