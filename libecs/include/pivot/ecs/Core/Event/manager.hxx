#pragma once

#include "pivot/ecs/Core/Data/value.hxx"
#include "pivot/ecs/Core/Event/description.hxx"
#include "pivot/ecs/Core/Systems/manager.hxx"
#include "pivot/ecs/Core/types.hxx"

#include <any>
#include <vector>

namespace pivot::ecs::event
{

class Manager
{
public:
    Manager(std::unique_ptr<systems::Manager> &systemManager): m_systemManager(systemManager){};
    void registerEvent(const Description &description);

    void sendEvent(const std::string &eventName, const data::Value &value, const Entities &entities = {});
    void sendEvent(const Description &description, const data::Value &value, const Entities &entities = {});

private:
    std::unique_ptr<systems::Manager> &m_systemManager;
};

}    // namespace pivot::ecs::event