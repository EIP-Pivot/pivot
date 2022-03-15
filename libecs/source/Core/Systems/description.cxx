#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Systems/description.hxx>

#include <iostream>
#include <stdexcept>

namespace pivot::ecs::systems
{

void Description::validate() const
{
    if (this->name.empty()) { throw ValidationError("Empty system name"); }

    if (this->systemComponents.empty()) { throw ValidationError("Empty system argument"); }

    this->eventListener.validate();

    if (this->eventComponents.size() != this->eventListener.entities.size())
        throw ValidationError("Event require " + std::to_string(this->eventListener.entities.size()) +
                              " entities given " + std::to_string(this->eventComponents.size()));
}

}    // namespace pivot::ecs::systems