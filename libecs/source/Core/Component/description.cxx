#include <magic_enum.hpp>
#include <set>

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::ecs::component
{

void Description::validate() const
{
    if (this->name.empty()) { throw ValidationError("Empty component name"); }

    std::set<std::string> property_names;

    for (auto &[name, type]: this->properties) {
        if (name.empty()) { throw ValidationError("Empty property name"); }

        if (property_names.contains(name)) { throw ValidationError("Duplicate property name"); }

        if (!magic_enum::enum_contains<Description::Property::Type>(type)) {
            throw ValidationError("Unknown property type");
        }

        property_names.insert(name);
    }
}
}    // namespace pivot::ecs::component
