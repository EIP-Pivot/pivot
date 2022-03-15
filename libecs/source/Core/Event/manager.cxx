#include "pivot/ecs/Core/Event/manager.hxx"
#include "pivot/ecs/Core/Event/index.hxx"

namespace pivot::ecs::event {
    Manager::Manager(systems::Manager &systemManager) : m_systemManager(systemManager) {}

    void Manager::sendEvent(const Event &event) {
        if (event.payload.type() != event.description.payload)
            throw std::runtime_error("This event expect " + event.description.payload.toString());
        m_systemManager.execute(event);
    }

}    // namespace pivot::ecs::event