#include "pivot/ecs/Core/Event/manager.hxx"
#include "pivot/ecs/Core/Event/index.hxx"

namespace pivot::ecs::event
{

void Manager::registerEvent(const Description &description)
{
    
}

void Manager::sendEvent(const std::string &eventName, const data::Value &value, const std::vector<Entity> &entities)
{
    const auto &decription = GlobalIndex::getSingleton().getDescription(eventName).value();
    Event event {
        .payload = value,
    };
}

}