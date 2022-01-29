#include "pivot/ecs/Core/Event/manager.hxx"
#include "pivot/ecs/Core/Event/index.hxx"

namespace pivot::ecs::event
{

void Manager::registerEvent(const Description &description)
{
    
}

void Manager::sendEvent(const std::string &eventName, const data::Value &payload, const std::vector<Entity> &entities)
{
    const auto description = GlobalIndex::getSingleton().getDescription(eventName).value();
    if (payload.type() != description.payload) throw std::logic_error("This event expect " + description.payload.toString());
    m_systemManager->execute(description, payload, entities);
}

}