#include "pivot/ecs/Core/EventManager.hxx"

void EventManager::AddListener(EventId eventId, std::function<void(Event &)> const &listener)
{
    listeners[eventId].push_back(listener);
}

void EventManager::SendEvent(Event &event)
{
    uint32_t type = event.GetType();

    for (auto const &listener: listeners[type]) { listener(event); }
}

void EventManager::SendEvent(EventId eventId)
{
    Event event(eventId);

    for (auto const &listener: listeners[eventId]) { listener(event); }
}