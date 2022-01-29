#pragma once

#include "pivot/ecs/Core/Component/manager.hxx"
#include "pivot/ecs/Core/EcsException.hxx"
#include "pivot/ecs/Core/EntityManager.hxx"
#include "pivot/ecs/Core/Systems/description.hxx"
#include "pivot/ecs/Core/Systems/index.hxx"

#include <tuple>
#include <functional>
#include <unordered_map>

namespace pivot::ecs::systems
{

class Manager
{
public:
    Manager(std::unique_ptr<component::Manager> &componentManager, std::unique_ptr<EntityManager> &entityManager)
        : m_componentManager(componentManager), m_entityManager(entityManager) {};
    
    bool useSystem(const std::string &systemName);
    bool useSystem(const Description &description);

    void execute(const event::Description &eventDescription, const data::Value &payload, const std::vector<Entity> &entities = {});

    std::vector<std::string> getSystemUsed();
private:
    std::vector<component::Manager::ComponentId> getComponentsId(const std::vector<std::string> &components);
    std::unique_ptr<component::Manager> &m_componentManager;
    std::unique_ptr<EntityManager> &m_entityManager;
    std::unordered_map<std::string, Description> m_systems;
};

}    // namespace pivot::ecs::systems