#include "pivot/ecs/Core/Event/manager.hxx"
#include "pivot/ecs/Core/Event/index.hxx"

namespace pivot::ecs::event
{
Manager::Manager(systems::Manager &systemManager): m_systemManager(systemManager) {}

void Manager::sendEvent(const Event &event)
{
    if (event.payload.type() != event.description.payload)
        throw std::runtime_error("This event expect " + event.description.payload.toString() + ", got " +
                                 event.payload.type().toString());
    for (const auto &childEvent: m_systemManager.execute(event)) this->sendEvent(childEvent);
}

}    // namespace pivot::ecs::event
