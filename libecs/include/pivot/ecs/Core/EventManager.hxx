#pragma once

#include "pivot/ecs/Core/types.hxx"
#include "pivot/ecs/Core/Event.hxx"
#include <functional>
#include <list>
#include <unordered_map>

/*! \cond
 */
class EventManager
{
public:
    void AddListener(EventId eventId, std::function<void(Event &)> const &listener);
    void SendEvent(Event &event);
    void SendEvent(EventId eventId);

private:
    std::unordered_map<EventId, std::list<std::function<void(Event &)>>> listeners;
};
/*! \endcond
 */