#include <magic_enum.hpp>
#include <set>

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::ecs::component
{

void Description::validate() const
{
    if (this->name.empty()) { throw ValidationError("Empty component name"); }
    if (this->defaultValue.type() != this->type) { throw ValidationError("Invalid default value"); }
}

bool Description::operator==(const Description &rhs) const
{
    return this->name == rhs.name && this->type == rhs.type && &this->createContainer == &rhs.createContainer;
}
}    // namespace pivot::ecs::component
