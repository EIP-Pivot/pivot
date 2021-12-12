#pragma once

#include <set>
#include <stdexcept>

#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/array.hxx>

namespace pivot::ecs::component
{

class TagArray : public DenseTypedComponentArray<Tag>
{
public:
    TagArray(Description d): DenseTypedComponentArray<Tag>(d) {}

    void setValueForEntity(Entity entity, std::optional<std::any> value) override;

    class DuplicateEntityTag : public std::logic_error
    {
    public:
        DuplicateEntityTag(const std::string &tag): std::logic_error("Duplicate entity tag: " + tag) {}
    };

private:
    std::set<std::string> m_tag_names;
};

}    // namespace pivot::ecs::component
