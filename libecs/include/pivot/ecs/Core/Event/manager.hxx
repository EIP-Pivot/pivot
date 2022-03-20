#pragma once

#include "pivot/ecs/Core/Data/value.hxx"
#include "pivot/ecs/Core/Event/description.hxx"
#include "pivot/ecs/Core/Systems/manager.hxx"
#include "pivot/ecs/Core/types.hxx"

#include <any>
#include <vector>

namespace pivot::ecs::event
{

/** \brief Manages all the events in a Scene
 *
 * The Manager send event and execute systems listing this event
 */
class Manager
{
public:
    /// Manager constructor take the scene system manager to execute systems
    Manager(systems::Manager &systemManager);

    /// Send event with event::Event Object
    void sendEvent(const Event &event);

private:
    systems::Manager &m_systemManager;
};

}    // namespace pivot::ecs::event