#include <pivot/ecs/Core/Systems/description.hxx>
#include <stdexcept>
#include <iostream>

namespace pivot::ecs::systems
{

void Description::validate() const
{
    if (this->name.empty()) { throw ValidationError("Empty component name"); }
}



}    // namespace pivot::ecs::component