#include <pivot/ecs/Core/Component/description.hxx>
#include <set>

namespace pivot::ecs::component
{

void Description::validate() const
{
    if (this->name.empty()) { throw ValidationError("Empty component name"); }

    std::set<std::string> property_names;

    for (auto &[name, type]: this->properties) {
        if (name.empty()) { throw ValidationError("Empty property name"); }

        if (property_names.contains(name)) { throw ValidationError("Duplicate property name"); }

        const std::set<Description::Property::Type> all_property_types{
            Description::Property::Type::STRING,
            Description::Property::Type::NUMBER,
            Description::Property::Type::ASSET,
            Description::Property::Type::VEC3,
        };
        if (!all_property_types.contains(type)) { throw ValidationError("Unknown property type"); }

        property_names.insert(name);
    }
}
}    // namespace pivot::ecs::component
