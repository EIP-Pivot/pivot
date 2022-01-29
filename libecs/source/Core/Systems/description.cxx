#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Systems/description.hxx>

#include <stdexcept>
#include <iostream>

namespace pivot::ecs::systems
{

void Description::validate() const
{
    if (this->name.empty()) { throw ValidationError("Empty system name"); }

    if (this->systemComponents.empty()) { throw ValidationError("Empty system argument"); }

    for (const auto &arg: this->systemComponents) {
        if (!ecs::component::GlobalIndex::getSingleton().getDescription(arg).has_value())
            throw ValidationError("Component " + arg + " his not registered.");
    }
}

}    // namespace pivot::ecs::systems