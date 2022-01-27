#include <magic_enum.hpp>
#include <set>

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::ecs::component
{

void Description::validate() const
{
    if (this->name.empty()) { throw ValidationError("Empty component name"); }
}
}    // namespace pivot::ecs::component
