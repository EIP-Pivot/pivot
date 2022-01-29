#include "pivot/ecs/Core/Event/manager.hxx"
#include "pivot/ecs/Core/Event/index.hxx"

namespace pivot::ecs::event
{

void Manager::registerEvent(const Description &description)
{
    
}

void Manager::sendEvent(const std::string &eventName, const data::Value &payload, const Entities &entities)
{
    const auto &description = GlobalIndex::getSingleton().getDescription(eventName).value();
    Event event{
        .description = description,
        .entities = entities,
        .payload = payload,
    };
    m_systemManager->execute(event);
}

}