#include "pivot/ecs/Core/Event/description.hxx"

namespace pivot::ecs::event {

    void Description::validate() const {
        if (this->name.empty()) { throw ValidationError("Empty event name"); }
    }

}    // namespace pivot::ecs::event